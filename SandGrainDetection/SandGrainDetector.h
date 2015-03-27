#include <string>
#include <cmath> // floor()

// I/O
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

// Filters
#include "itkHMinimaImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryImageToLabelMapFilter.h"

// PointSet
#include "itkPointSet.h"


template <typename PixelType, uint32_t Dimension>
void detectSandGrains(const std::string inputFile, const std::string outputFile,
                      const double H, const double threshPerc)
{
    using ImageType = itk::Image<PixelType, Dimension>;

    // The max unsigned representable value by PixelType
    const PixelType pixelTypeMax = (1 << (sizeof(PixelType) * 8)) - 1;

    // Set and configure reader
    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(inputFile);
 
    // For the following filters, the parameter is configured with the input
    // param, which is a percentage (in range [0, 1]).
    // Set and configure HMinimaImageFilter
    auto minFilter = itk::HMinimaImageFilter<ImageType, ImageType>::New();
    const PixelType hIntensityUnits = PixelType( floor(H * pixelTypeMax) );
    std::cout << "hIntensityUnits = " << uint32_t(hIntensityUnits) << std::endl;
    minFilter->SetHeight(hIntensityUnits);
    minFilter->SetInput(reader->GetOutput());

    // Set and configure BinaryThreshold filter. This will binarize the image
    // so that anything below threshVal will be put to 0, and anything above
    // it will be put to pixelTypeMax
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
    auto labelMapFilter = itk::BinaryImageToLabelMapFilter<ImageType>::New();
    labelMapFilter->SetInput(thresholdFilter->GetOutput());
    labelMapFilter->Update();

    // Run pipeline
    labelMapFilter->Update();

    
    // Change to PointSet
    // Some helpful aliases
    using PointSetType = itk::PointSet<PixelType, Dimension>;
    using IteratorType = itk::ImageRegionConstIterator<ImageType>;
    using PointType = typename PointSetType::PointType;

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

    std::cout << "Number of points: " << pointSet->GetNumberOfPoints() << std::endl;

    // Segment?

    // Compute bounding boxes

    // Write to PCL-compatible output

    // Set and configure writer
    auto writer = itk::ImageFileWriter<ImageType>::New();
    writer->SetFileName(outputFile);
    writer->SetInput(thresholdFilter->GetOutput());

    // Execute pipeline
    writer->Update();

    // Print out number of labels
    std::cout << "There are "
              << labelMapFilter->GetOutput()->GetNumberOfLabelObjects()
              << " objects." << std::endl;

}
