#include <unistd.h>
#include <string>
#include <cmath>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
// Filters
#include "itkHMinimaImageFilter.h"
#include "itkThresholdImageFilter.h"
// PointSet
#include "itkPointSet.h"
#include "itkImageRegionConstIterator.h"
 
const uint32_t Dimension = 3;
typedef uint8_t PixelType;
typedef itk::Image<PixelType, Dimension> ImageType;
 
int main(int argc, char **argv)
{
    // Verify number of params and parse and validate args
    if (argc < 5) {
        std::cout << "Usage: " << argv[0] << " inputImage outputImage H threshVal" << std::endl; 
        exit(1);
    }

    std::string inputFilename = std::string(argv[1]);
    if (access(inputFilename.c_str(), F_OK) == -1) {
        std::cerr << "[error]: " << inputFilename << " does not exist" << std::endl;
        exit(1);
    }
    std::string outputFilename = std::string(argv[2]);
    const double H = std::stod(argv[3]);
    const double threshPerc = std::stod(argv[4]);

    // Read in image
    //const ImageType* im = readImage<ImageType>(inputFilename);

    // Apply HMinimaImageFilter
    //const ImageType* afterMorphology = applyMorphologyFilter<ImageType>(H, im);

    // Apply ThresholdFilter
    //const ImageType* afterThreshold = applyThreshold<ImageType>(threshPerc, afterMorphology);

    // Change to PointSet

    // Segment?

    // Compute bounding boxes

    // Write to PCL-compatible output

    // Set and configure reader
    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(inputFilename);
 
    // For the following filters, the parameter is configured with the input param, which is
    // a percentage (in range [0, 1]).
    // Set and configure HMinimaImageFilter
    auto minFilter = itk::HMinimaImageFilter<ImageType, ImageType>::New();
    PixelType hIntensityUnits = PixelType(floor(H * ((1 << (sizeof(PixelType) * 8)) - 1)));
    minFilter->SetHeight(hIntensityUnits);
    minFilter->SetInput(reader->GetOutput());

    // Set and configure Threshold filter
    auto thresholdFilter = itk::ThresholdImageFilter<ImageType>::New();
    thresholdFilter->SetInput(minFilter->GetOutput());
    thresholdFilter->SetOutsideValue(0);
    PixelType threshVal = PixelType(floor(threshPerc * ((1 << (sizeof(PixelType) * 8)) - 1)));
    thresholdFilter->ThresholdBelow(threshVal);

    // Set and configure writer
    auto writer = itk::ImageFileWriter<ImageType>::New();
    writer->SetFileName(outputFilename);
    writer->SetInput(thresholdFilter->GetOutput());

    // Execute pipeline
    try {
        writer->Update();
    } catch (itk::ExceptionObject& err) {
        std::cerr << "ExceptionObject caught!" << std::endl;
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }
 
    return EXIT_SUCCESS;
}
