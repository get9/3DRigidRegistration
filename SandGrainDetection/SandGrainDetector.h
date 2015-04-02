#include <string>
#include <cmath> // floor()
#include <limits> // numeric_limits<T>::max()

// I/O
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

// Filters
#include "itkMultiScaleHessianBasedMeasureFilter.h"
#include "itkHessianToObjectnessMeasureImageFilter.h"
#include "itkRescaleIntensityFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryImageToShapeLabelMapFilter.h"

// PointSet
#include "itkPointSet.h"


template <typename PixelType, uint32_t Dimension>
void detectSandGrains(const std::string inputFile, const std::string outputFile,
                      const double H, const double threshPerc)
{
    using ImageType = itk::Image<PixelType, Dimension>;

    // The max unsigned representable value by PixelType
    const PixelType pixelTypeMax = std::numeric_limits<PixelType>::max();

    // Set and configure reader
    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(inputFile);
 
    using HessianPixelType = itk::SymmetricSecondRankTensor<double, Dimension>;
    using HessianImageType = itk::Image<HessianPixeltype, Dimension>;
    using ObjectnessFilterType =
        itk::HessiantoObjectnessMeasureImageFilter<HessianImageType, ImageType>;
    auto objectnessFilter = ObjectnessFilterType::New();
    objectnessFilter->SetBrightObject(true);
    objectnessFilter->SetScaleObjectnessMeasure(false);
    objectnessFilter->SetAlpha(0.5);
    objectnessFilter->SetBeta(1.0);
    objectnessFilter->SetGamma(5.0);

    using MultiScaleEnhancementFilterType =
        itk::MultiScaleHessianBasedMeasureFilter<ImageType, HessianImageType, ImageType>::New();
    const double sigmaMinimum = 1.0;
    const double sigmaMaximum = 10.0;
    const uint32_t numberOfSigmaSteps = 10;
    auto multiScaleEnhancementFilter = MultiScaleEnhancementFilter::New();
    multiScaleEnhancementFilter->SetInput(reader->GetOutput());
    multiScaleEnhancementFilter->SetHessianToMeasureFilter(objectnessFilter);
    multiScaleEnhancementFilter->SetSigmaStepMethodToLogarithmic();
    multiScaleEnhancementFilter->SetSigmaMinimum(sigmaMinimum);
    multiScaleEnhancementFilter->SetSigmaMaximum(sigmaMaximum);
    multiScaleEnhancementFilter->SetNumberOfSigmaSteps(numberOfSigmaSteps);
    // 0 for detecting blobs
    multiScaleEnhancementFilter->SetObjectDimension(0);

    // Set and configure BinaryThreshold filter. This will binarize the image
    // so that anything below threshVal will be put to 0, and anything above
    // it will be put to pixelTypeMax
    /*
    auto thresholdFilter = itk::BinaryThresholdImageFilter<ImageType, ImageType>::New();
    thresholdFilter->SetInput(minFilter->GetOutput());
    thresholdFilter->SetOutsideValue(0);
    thresholdFilter->SetInsideValue(pixelTypeMax);
    const PixelType threshVal = PixelType( floor(threshPerc * pixelTypeMax) );
    std::cout << "threshVal = " << uint32_t(threshVal) << std::endl;
    thresholdFilter->SetLowerThreshold(threshVal);
    thresholdFilter->SetUpperThreshold(pixelTypeMax);

    // Run binary image to label filter. This will label all the regions that
    // are non-zero (due to thresholding).
    auto labelMapFilter = itk::BinaryImageToShapeLabelMapFilter<ImageType>::New();
    labelMapFilter->SetInput(thresholdFilter->GetOutput());

    // Run pipeline
    labelMapFilter->Update();

    // Add centroids of grains to PointSet
    // XXX start at 1 because label 0 is the background object (i.e. all black)
    using PointSetType = itk::PointSet<PixelType, Dimension>;
    using IteratorType = itk::ImageRegionConstIterator<ImageType>;
    using PointType = typename PointSetType::PointType;

    auto pointSet = itk::PointSet<PixelType, Dimension>::New();
    const auto labelMapFilterOutput = labelMapFilter->GetOutput();
    for (uint32_t i = 1; i <= labelMapFilterOutput->GetNumberOfLabelObjects(); ++i) {
        const auto shapeLabelObject = labelMapFilterOutput->GetLabelObject(i);
        std::cout << i << "\t" << shapeLabelObject->GetCentroid() << std::endl;
        pointSet->SetPoint(i - 1, shapeLabelObject->GetCentroid());
    }

    //std::cout << pointSet << std::endl;
    */

    /*
    // The image we're iterating over and it's associated iterator
    auto image = thresholdFilter->GetOutput();
    IteratorType it(image, image->GetRequestedRegion());
    it.GoToBegin();

    PointType point;
    uint64_t pointID = 0;
    auto pointSet = PointSetType::New();

    // Iterate through image, only taking non-0 intensity-value pixels
    while ( !it.IsAtEnd() ) {
        // Only transform index to physical coords if pixel isn't black
        // (otherwise, we don't care about it)
        if (it.Get() != 0) {
            // Convert pixel index into a physical point
            image->TransformIndexToPhysicalPoint(it.GetIndex(), point);
            std::cout << uint32_t(it.Get()) << " --> " << point << std::endl;
            pointSet->SetPoint(pointID, point);
            pointSet->SetPointData(pointID, it.Get());
            ++pointID;
        }
        ++it;
    }

    // Write PointSet to just a text file
    auto points = pointSet->GetPoints();
    auto iter = points->Begin();
    while (iter != points->End()) {

    }

    // Segment?

    // Compute bounding boxes

    // Write to PCL-compatible output
    */

    // Set and configure writer
    auto writer = itk::ImageFileWriter<ImageType>::New();
    writer->SetFileName(outputFile);
    writer->SetInput(multiScaleEnhancementFilter->GetOutput());
    writer->Update();

    // Print out number of labels
    std::cout << "There are "
              << labelMapFilter->GetOutput()->GetNumberOfLabelObjects()
              << " objects." << std::endl;

}
