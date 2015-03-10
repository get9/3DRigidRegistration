#include <unistd.h>
#include <string>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkShrinkImageFilter.h"
 
const unsigned int Dimension = 3;
typedef unsigned char PixelType;
typedef itk::Image<PixelType, Dimension> ImageType;
 
int main(int argc, char **argv)
{
    // Verify number of params and parse and validate args
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " shrinkFactor" << " inputImage"
                  << " outputImage" << std::endl; 
        exit(1);
    }
    double shrinkFactor = 0.0f;
    try {
        shrinkFactor = std::stod(argv[1]);
    } catch (const std::invalid_argument& arg) {
        std::cerr << "[error]: shrinkFactor must be a number" << std::endl;
        exit(1);
    }
    if (shrinkFactor < 1.0f) {
        std::cerr << "[error]: shrinkFactor must be in the range [1, inf) as a"
                  << " shrink factor of size of the old volume";
        exit(1);
    }
    const char* inputImageFileName = argv[2];
    if (access(inputImageFileName, F_OK) == -1) {
        std::cerr << "[error]: " << inputImageFileName << " does not exist"
                  << std::endl;
        exit(1);
    }
    const char* outputImageFileName = argv[3];

    // Set and configure reader
    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(inputImageFileName);
    reader->GenerateOutputInformation();
    std::cout << "Original size: "
              << reader->GetOutput()->GetLargestPossibleRegion().GetSize()
              << std::endl;
 
    // Set and configure ShrinkImageFilter
    auto shrinkFilter = itk::ShrinkImageFilter<ImageType, ImageType>::New();
    shrinkFilter->SetInput(reader->GetOutput());
    for (uint32_t i = 0; i < Dimension; ++i) {
        shrinkFilter->SetShrinkFactor(i, shrinkFactor);
    }

    // Run shrink filter
    try {
        shrinkFilter->Update();
    } catch (itk::ExceptionObject& err) {
        std::cerr << "ExceptionObject caught!" << std::endl;
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }
 
    std::cout << "New size: "
              << shrinkFilter->GetOutput()->GetLargestPossibleRegion().GetSize()
              << std::endl;
 
    return EXIT_SUCCESS;
}
