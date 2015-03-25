#include <string>
#include <cmath> // floor()

// I/O
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

// Filters
#include "itkHMinimaImageFilter.h"
#include "itkThresholdImageFilter.h"


template <typename PixelType, uint32_t Dimension>
void detectSandGrains(const std::string inputFile, const std::string outputFile,
                      const double H, const double threshPerc)
{
    using ImageType = itk::Image<PixelType, Dimension>;

    // Set and configure reader
    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(inputFile);
 
    // For the following filters, the parameter is configured with the input
    // param, which is a percentage (in range [0, 1]).
    // Set and configure HMinimaImageFilter
    auto minFilter = itk::HMinimaImageFilter<ImageType, ImageType>::New();
    PixelType hIntensityUnits = PixelType(
        floor(H * ((1 << (sizeof(PixelType) * 8)) - 1))
    );
    std::cout << "hIntensityUnits = " << hIntensityUnits << std::endl;
    minFilter->SetHeight(hIntensityUnits);
    minFilter->SetInput(reader->GetOutput());

    // Set and configure Threshold filter
    auto thresholdFilter = itk::ThresholdImageFilter<ImageType>::New();
    thresholdFilter->SetInput(minFilter->GetOutput());
    thresholdFilter->SetOutsideValue(0);
    PixelType threshVal = PixelType(
        floor(threshPerc * ((1 << (sizeof(PixelType) * 8)) - 1))
    );
    std::cout << "threshVal = " << threshVal << std::endl;
    thresholdFilter->ThresholdBelow(threshVal);

    // Change to PointSet

    // Segment?

    // Compute bounding boxes

    // Write to PCL-compatible output

    // Set and configure writer
    auto writer = itk::ImageFileWriter<ImageType>::New();
    writer->SetFileName(outputFile);
    writer->SetInput(thresholdFilter->GetOutput());

    // Execute pipeline
    writer->Update();
}
