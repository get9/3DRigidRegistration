#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"

template <typename InPixel, typename OutPixel>
const bool convertBitDepth(const std::string infile, const std::string outfile)
{
    const uint32_t Dimension = 3;
    typedef itk::Image< InPixel, Dimension > InputPixelType;
    typedef itk::Image< OutPixel, Dimension > OutputPixelType;
    typedef itk::ImageFileReader< InputPixelType > ReaderType;
    typedef itk::ImageFileWriter< OutputPixelType > WriterType;

    // Set up reader and writer
    auto reader = ReaderType::New();
    auto writer = WriterType::New();
    reader->SetFileName(infile);
    writer->SetFileName(outfile);

    // Set up rescaler
    typedef itk::RescaleIntensityImageFilter< InputPixelType, OutputPixelType > RescaleType;
    auto rescaler = RescaleType::New();
    // Since we're only ever dealing with 8/16 bits, 32-bit should be big enough to hold new values
    const uint32_t inBits = sizeof(InPixel) * 8;
    const uint32_t outBits = sizeof(OutPixel) * 8;
    rescaler->SetOutputMinimum(0);
    rescaler->SetOutputMaximum((1 << outBits) - 1);
    rescaler->SetInput(reader->GetOutput());

    // Connect writer to the rescaler
    writer->SetInput(rescaler->GetOutput());

    // Run the pipeline
    try {
        writer->Update();
    } catch (itk::ExceptionObject& ex) {
        std::cerr << "[error]: ExceptionObject caught" << std::endl;
        std::cerr << ex << std::endl;
        return false;
    }
    return true;
}
