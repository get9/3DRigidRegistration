#include <string>
#include <unistd.h>
#include <cmath>

#include "itkImage.h"
#include "itkImageSeriesReader.h"
#include "itkImageFileWriter.h"
#include "itkNumericSeriesFileNames.h"
#include "itkTIFFImageIO.h"

size_t get_memsize(void);

template <typename PixelType>
bool doConvert(itk::NumericSeriesFileNames::Pointer generator,
               const std::string outputFilename, const float memUsageCoefficient)
{
    auto reader = itk::ImageSeriesReader<itk::Image<PixelType, 3>>::New();
    auto writer = itk::ImageFileWriter<itk::Image<PixelType, 3>>::New();

    reader->SetImageIO(itk::TIFFImageIO::New());
    reader->SetFileNames(generator->GetFileNames());
    // Enable streaming reads so we don't read the whole thing into memory
    reader->SetUseStreaming(true);
    reader->GenerateOutputInformation();

    // Compute appropriate number of divisions based on requested memory size
    auto fullsize = reader->GetOutput()->GetLargestPossibleRegion().GetSize();
    uint64_t volumeSize = fullsize[0] * fullsize[1] * fullsize[2];
    auto streamDivisions = (uint32_t) ceil(volumeSize / (memUsageCoefficient * get_memsize()));

    writer->SetFileName(outputFilename);
    writer->SetInput(reader->GetOutput());
    writer->SetNumberOfStreamDivisions(streamDivisions);

    try {
        writer->Update();
    } catch( itk::ExceptionObject & err ) {
        std::cerr << "ExceptionObject caught !" << std::endl;
        std::cerr << err << std::endl;
        return false;
    }
    return true;
}

int main( int argc, char ** argv )
{
    // Verify the number of parameters in the command line
    if (argc < 8) {
        std::cerr << "Usage: " << std::endl;
        std::cerr << argv[0] << " inputDirectory"
                  << " filenameFormat" << " bitdepth"
                  << " firstSlice" << " lastSlice"
                  << " outputImageFile" << " memCoefficient"
                  << std::endl;
        return EXIT_FAILURE;
    }

    // Dimension is constant at 3 since we're dealing with volumes
    const uint32_t Dimension = 3;

    // Parse input parameters
    const std::string inputDirectory = std::string(argv[1]);
    const std::string filenameFormat = std::string(argv[2]);
    const uint32_t bitdepth = std::stoi(argv[3]);
    if (bitdepth != 8 && bitdepth != 16) {
        std::cerr << "[error]: bitDepth must be one of {8, 16}" << std::endl;
        return EXIT_FAILURE;
    }
    const uint32_t first = std::stoi(argv[4]);
    const uint32_t last  = std::stoi(argv[5]);
    const std::string outputFilename = std::string(argv[6]);
    const float memUsage = std::stof(argv[7]);

    // Making the name generator
    std::string inputFiles = inputDirectory + "/" + filenameFormat;
    auto nameGenerator = itk::NumericSeriesFileNames::New();
    nameGenerator->SetSeriesFormat(inputFiles.c_str()); 
    nameGenerator->SetStartIndex(first);
    nameGenerator->SetEndIndex(last);
    nameGenerator->SetIncrementIndex(1);

    // Instantiate readers and writers based on 8-bit or 16-bit depth
    if (bitdepth == 8) {
        return (!doConvert<uint8_t>(nameGenerator, outputFilename, memUsage) ? EXIT_FAILURE : EXIT_SUCCESS);
    } else {
        return (!doConvert<uint16_t>(nameGenerator, outputFilename, memUsage) ? EXIT_FAILURE : EXIT_SUCCESS);
    }
}

// Get the system memory size for stream division configuration
size_t get_memsize(void)
{
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size;
}
