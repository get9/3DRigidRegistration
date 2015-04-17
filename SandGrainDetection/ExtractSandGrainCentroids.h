#include <string>
#include <iostream> // cout, endl, ofstream
#include <cmath> // floor()
#include <limits> // numeric_limits<T>::max()

// I/O
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

// Filters
#include "itkHMinimaImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryImageToShapeLabelMapFilter.h"
#include "itkRescaleIntensityImageFilter.h"

// PointSet
#include "itkPointSet.h"
#include "PointSetUtil.h"


template <typename TPixel, uint32_t TDimension>
void extractSandGrainCentroids(const std::string inputFile, const std::string outputFile,
                               const std::string pointsFile, const double H, const double threshPerc)
{
    using ImageType = itk::Image<TPixel, TDimension>;

    // The max unsigned representable value by TPixel
    const TPixel pixelMax = std::numeric_limits<TPixel>::max();

    // Set and configure reader
    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(inputFile);
 
    // After running morphology filter, need to rescale the intensities
    //auto rescaler = itk::RescaleIntensityImageFilter<ImageType, ImageType>::New();
    //rescaler->SetInput(reader->GetOutput());

    // For the following filters, the parameter is configured with the input
    // param, which is a percentage (in range [0, 1]).
    // Set and configure HMinimaImageFilter
    auto convexFilter = itk::HMinimaImageFilter<ImageType, ImageType>::New();
    const TPixel hIntensityUnits = TPixel( floor(H * pixelMax) );
    std::cout << "hIntensityUnits = " << uint32_t(hIntensityUnits) << std::endl;
    convexFilter->SetHeight(hIntensityUnits);
    convexFilter->SetInput(reader->GetOutput());

    // Set and configure BinaryThreshold filter. This will binarize the image
    // so that anything below threshVal will be put to 0, and anything above
    // it will be put to pixelMax
    auto thresholdFilter = itk::BinaryThresholdImageFilter<ImageType, ImageType>::New();
    thresholdFilter->SetInput(convexFilter->GetOutput());
    thresholdFilter->SetOutsideValue(0);
    thresholdFilter->SetInsideValue(pixelMax);
    const TPixel threshVal = TPixel( floor(threshPerc * pixelMax) );
    std::cout << "threshVal = " << uint32_t(threshVal) << std::endl;
    thresholdFilter->SetLowerThreshold(threshVal);
    thresholdFilter->SetUpperThreshold(pixelMax);

    // Write out the thresholded and morphed image to file so we can observe how it worked
    auto outputImageWriter = itk::ImageFileWriter<ImageType>::New();
    outputImageWriter->SetInput(thresholdFilter->GetOutput());
    outputImageWriter->SetFileName(outputFile);
    try {
        outputImageWriter->Update();
    } catch (itk::ExceptionObject& ex) {
        std::cerr << "Caught itk::ExceptionObject" << std::endl;
        std::cerr << ex << std::endl;
        exit(1);
    }

    // Run binary image to label filter. This will label all the regions that
    // are non-zero (due to thresholding).
    auto labelMapFilter = itk::BinaryImageToShapeLabelMapFilter<ImageType>::New();
    labelMapFilter->SetInput(thresholdFilter->GetOutput());

    // Run pipeline
    labelMapFilter->Update();

    // Change labeled regions into a PointSet
    auto pointSet = itk::PointSet<double, TDimension>::New();
    const auto labelMapFilterOutput = labelMapFilter->GetOutput();
    // XXX start at 1 because label 0 is the background object (i.e. all black)
    for (auto i = 1, pointCount = 0; i <= labelMapFilterOutput->GetNumberOfLabelObjects(); ++i) {
        const auto shapeLabelObject = labelMapFilterOutput->GetLabelObject(i);
        // Skip this ShapeLabelObject if there's only one pixel in it (too small to consider)
        if (shapeLabelObject->GetNumberOfPixels() < 2) {
            continue;
        }
        std::cout << shapeLabelObject->GetCentroid() << std::endl;
        pointSet->SetPoint(pointCount, shapeLabelObject->GetCentroid());
        pointCount++;
    }

    // Write pointset to file
    if (writeToFile<double, TDimension>(pointsFile, pointSet) < 0) {
        std::cerr << "[error]: could not write to file " << outputFile << std::endl;
        exit(1);
    }
}
