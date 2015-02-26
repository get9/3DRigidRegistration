#include <string>
#include <unistd.h>
#include <cmath>

#include "itkImage.h"
#include "itkImageSeriesReader.h"
#include "itkImageFileWriter.h"
#include "itkNumericSeriesFileNames.h"
#include "itkTIFFImageIO.h"

size_t get_memsize(void);

int main( int argc, char ** argv )
{
    // Verify the number of parameters in the command line
    if (argc < 6) {
        std::cerr << "Usage: " << std::endl;
        std::cerr << argv[0] << " inputDirectory" << " firstSliceValue"
                  << " lastSliceValue" << " outputImageFile"
                  << " memCoefficient" << std::endl;
        return EXIT_FAILURE;
    }

    typedef unsigned char PixelType;
    const unsigned int Dimension = 3;

    typedef itk::Image<PixelType, Dimension> ImageType;
    auto reader = itk::ImageSeriesReader<ImageType>::New();
    auto writer = itk::ImageFileWriter<ImageType>::New();

    // Parse input parameters
    const std::string inputDirectory = std::string(argv[1]);
    const unsigned int first = std::stoi(argv[2]);
    const unsigned int last  = std::stoi(argv[3]);
    const char * outputFilename = argv[4];
    const float memUsageCoefficient = std::stof(argv[5]);

    auto nameGenerator = itk::NumericSeriesFileNames::New();

    // Set up name generator to get all file names
    std::string filenameFormat = inputDirectory + "/" +
                                 "PHerc3-239um-OversizeOffset-%04d.tif";
    nameGenerator->SetSeriesFormat( filenameFormat.c_str() ); 
    nameGenerator->SetStartIndex(first);
    nameGenerator->SetEndIndex(last);
    nameGenerator->SetIncrementIndex(1);

    reader->SetImageIO( itk::TIFFImageIO::New() );
    reader->SetFileNames( nameGenerator->GetFileNames() );
    // Enable streaming reads so we don't read the whole thing into memory
    reader->SetUseStreaming(true);
    reader->GenerateOutputInformation();

    // Compute appropriate number of divisions based on requested memory size
    ImageType::SizeType fullsize = reader->GetOutput()->GetLargestPossibleRegion().GetSize();
    auto volumeSize = fullsize[0] * fullsize[1] * fullsize[2];
    auto streamDivisions = (uint32_t) ceil(volumeSize / (memUsageCoefficient * get_memsize()));

    writer->SetFileName( outputFilename );
    writer->SetInput( reader->GetOutput() );
    writer->SetNumberOfStreamDivisions(streamDivisions);

    try {
        writer->Update();
    } catch( itk::ExceptionObject & err ) {
        std::cerr << "ExceptionObject caught !" << std::endl;
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// Get the system memory size for stream division configuration
size_t get_memsize(void)
{
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size;
}
