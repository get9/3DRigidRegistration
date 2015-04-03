#include <string>
#include <iostream> // cout, endl, ofstream
#include <cmath> // floor()
#include <limits> // numeric_limits<T>::max()

// I/O
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

// Filters
#include "itkHConvexImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryImageToShapeLabelMapFilter.h"
#include "itkRescaleIntensityImageFilter.h"

// PointSet
#include "itkPointSet.h"
#include "PointSetUtil.h"


template <typename TPixel, uint32_t TDimension>
void extractSandGrainCentroids(const std::string inputFile, const std::string outputFile,
                               const double H, const double threshPerc)
{
    using ImageType = itk::Image<TPixel, TDimension>;

    // The max unsigned representable value by TPixel
    const TPixel pixelMax = std::numeric_limits<TPixel>::max();

    // Set and configure reader
    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(inputFile);
 
    // For the following filters, the parameter is configured with the input
    // param, which is a percentage (in range [0, 1]).
    // Set and configure HMinimaImageFilter
    auto convexFilter = itk::HConvexImageFilter<ImageType, ImageType>::New();
    const TPixel hIntensityUnits = TPixel( floor(H * pixelMax) );
    std::cout << "hIntensityUnits = " << uint32_t(hIntensityUnits) << std::endl;
    convexFilter->SetHeight(hIntensityUnits);
    convexFilter->SetInput(reader->GetOutput());

    // After running morphology filter, need to rescale the intensities
    auto rescaler = itk::RescaleIntensityImageFilter<ImageType, ImageType>::New();
    rescaler->SetInput(convexFilter->GetOutput());

    // Set and configure BinaryThreshold filter. This will binarize the image
    // so that anything below threshVal will be put to 0, and anything above
    // it will be put to pixelMax
    auto thresholdFilter = itk::BinaryThresholdImageFilter<ImageType, ImageType>::New();
    thresholdFilter->SetInput(rescaler->GetOutput());
    thresholdFilter->SetOutsideValue(0);
    thresholdFilter->SetInsideValue(pixelMax);
    const TPixel threshVal = TPixel( floor(threshPerc * pixelMax) );
    std::cout << "threshVal = " << uint32_t(threshVal) << std::endl;
    thresholdFilter->SetLowerThreshold(threshVal);
    thresholdFilter->SetUpperThreshold(pixelMax);

    // Run binary image to label filter. This will label all the regions that
    // are non-zero (due to thresholding).
    auto labelMapFilter = itk::BinaryImageToShapeLabelMapFilter<ImageType>::New();
    labelMapFilter->SetInput(thresholdFilter->GetOutput());

    // Run pipeline
    labelMapFilter->Update();

    auto pointSet = itk::PointSet<double, TDimension>::New();
    const auto labelMapFilterOutput = labelMapFilter->GetOutput();
    // XXX start at 1 because label 0 is the background object (i.e. all black)
    for (uint32_t i = 1; i <= labelMapFilterOutput->GetNumberOfLabelObjects(); ++i) {
        const auto shapeLabelObject = labelMapFilterOutput->GetLabelObject(i);
        pointSet->SetPoint(i - 1, shapeLabelObject->GetCentroid());
    }

    // Write pointset to file
    if (writeToFile<double, TDimension>(outputFile, pointSet) < 0) {
        std::cerr << "[error]: could not write to file " << outputFile << std::endl;
        exit(1);
    }
}
