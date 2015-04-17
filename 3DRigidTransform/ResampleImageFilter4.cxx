#include <string>
#include <cmath>
#include <cstdlib>
#include <limits>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkPasteImageFilter.h"
#include "itkSubtractImageFilter.h"

inline double deg2rad(double angle)
{
    return angle * (M_PI / 180.0);
}

int main( int argc, char * argv[] )
{
    if( argc < 4 ) {
        std::cerr << "Usage: " << std::endl;
        std::cerr << argv[0] << " fixed moving newimage tmpimage" << std::endl;
        return EXIT_FAILURE;
    }

    const uint32_t Dimension = 3;
    using TPixel = uint8_t;
    using TImage = itk::Image<TPixel, Dimension>;

    // Read args
    const auto fixedFilename  = std::string(argv[1]);
    const auto movingFilename = std::string(argv[2]);
    const auto outputFilename = std::string(argv[3]);
    const auto tmpFilename    = std::string(argv[4]);

    // Set up reader
    auto fixedReader  = itk::ImageFileReader<TImage>::New();
    fixedReader->SetFileName(fixedFilename);
    auto movingReader = itk::ImageFileReader<TImage>::New();
    movingReader->SetFileName(movingFilename);

    // Get parameters of new image to construct and make it
    fixedReader->Update();
    auto newImageSize = fixedReader->GetOutput()->GetLargestPossibleRegion().GetSize();
    auto newImage = itk::Image<TPixel, Dimension>::New();
    TImage::IndexType origin;
    origin.Fill(0);
    TImage::RegionType region(origin, newImageSize);
    newImage->SetRegions(region);
    newImage->Allocate();

    // Configure region where we will paste it
    movingReader->Update();
    fixedReader->Update();
    auto movingSize = movingReader->GetOutput()->GetLargestPossibleRegion().GetSize();
    auto fixedSize = fixedReader->GetOutput()->GetLargestPossibleRegion().GetSize();
    TImage::IndexType start;
    start[0] = uint32_t(floor((fixedSize[0] * 0.45 - movingSize[0]/2)));
    start[1] = uint32_t(floor((fixedSize[1] * 0.55 - movingSize[1]/2)));
    start[2] = 107;

    // Paste image region onto new image
    auto pasteFilter = itk::PasteImageFilter<TImage, TImage>::New();
    pasteFilter->SetSourceImage(movingReader->GetOutput());
    pasteFilter->SetSourceRegion(movingReader->GetOutput()->GetLargestPossibleRegion());
    pasteFilter->SetDestinationImage(newImage);
    pasteFilter->SetDestinationIndex(start);

    // Write the pasted file to an intermediate file
    auto intermediateWriter = itk::ImageFileWriter<TImage>::New();
    intermediateWriter->SetFileName(tmpFilename);
    intermediateWriter->SetInput(pasteFilter->GetOutput());
    intermediateWriter->Update();

    // Now subtract the fixed image and the new image created by the paste filter
    auto subtractFilter = itk::SubtractImageFilter<TImage, TImage, TImage>::New();
    subtractFilter->SetInput1(fixedReader->GetOutput());
    subtractFilter->SetInput2(pasteFilter->GetOutput());

    auto writer = itk::ImageFileWriter<TImage>::New();
    writer->SetFileName(outputFilename);
    writer->SetInput(subtractFilter->GetOutput());

    //    The output of the resampling resampleFilter is connected to a writer and the
    //    execution of the pipeline is triggered by a writer update.
    try {
        writer->Update();
    }
    catch( itk::ExceptionObject & excep ) {
        std::cerr << "Exception caught !" << std::endl;
        std::cerr << excep << std::endl;
    }

    return EXIT_SUCCESS;
}
