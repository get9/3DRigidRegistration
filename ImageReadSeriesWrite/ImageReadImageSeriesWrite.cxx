#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageSeriesWriter.h"
#include "itkNumericSeriesFileNames.h"


const uint32_t MhdDimension = 3;
const uint32_t SliceDimension = 2;

template <typename PixelType>
bool doConvert(std::string inputFile, std::string outputDir)
{
    // Create reader
    typedef itk::Image<Pixeltype, MhdDimension> ImageType;
    typedef itk::ImageFileReader<ImageType> ReaderType;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(inputFile);

    // Create writer, connect to reader
    typedef itk::Image<PixelType, SliceDimension> Image2DType;
    typedef itk::ImageSeriesWriter<ImageType, Image2DType> WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetInput(reader->GetOutput());

    // Execute read
    try {
        reader->Update();
    }
    catch (itk::ExceptionObject& excp) {
        std::cerr << "Exception thrown while reading the image" << std::endl;
        std::cerr << excp << std::endl;
        return false;
    }

    // Get slice start/end from read image
    const auto region = reader->GetOutput()->GetLargestPossibleRegion()
    const auto start = region.GetIndex();
    const auto size = region.GetSize();
    const uint32_t firstSlice = start[2];
    const uint32_t lastSlice = start[2] + size[2] - 1;

    // Set up name generator. Outputs names like 0000.tif, 0001.tif, ...
    typedef itk::NumericSeriesFileNames NameGeneratorType;
    NameGeneratorType::Pointer nameGenerator = NameGeneratorType::New();
    const std::string format = outputDir + "/" + "%04d.tif";
    nameGenerator->SetSeriesFormat(format.c_str());
    nameGenerator->SetStartIndex(firstSlice);
    nameGenerator->SetEndIndex(lastSlice);
    nameGenerator->SetIncrementIndex(1);
    writer->SetFileNames(nameGenerator->GetFileNames());

    // Execute write
    try {
        writer->Update();
    }
    catch (itk::ExceptionObject& excp) {
        std::cerr << "Exception thrown while reading the image" << std::endl;
        std::cerr << excp << std::endl;
        return false;
    }
    return true;
}

int main( int argc, char *argv[] )
{
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0]
                  << " inputFile outputDir bitDepth"
                  << std::endl;
        return EXIT_FAILURE;
    }

    // Parse/validate args
    const std::string inputFile = std::string(argv[1]);
    const std::string outputDir = std::string(argv[2]);
    const uint32_t bitdepth = std::stoi(argv[3]);
    if (bitdepth != 8 && bitdepth != 16) {
        std::cerr << "[error]: bitDepth must be one of {8, 16}" << std::endl;
        return EXIT_FAILURE;
    }

    // Run conversion, switching based on passed bitdepth value
    bool didConvert = false;
    if (bitdepth == 8) {
        didConvert = doConvert<uint8_t>(inputFile, outputDir);
    } else {
        didConvert = doConvert<uint16_t>(inputFile, outputDir);
    }

    return (didConvert ? EXIT_SUCCESS : EXIT_FAILURE);
}
