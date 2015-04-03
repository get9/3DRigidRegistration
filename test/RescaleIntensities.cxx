#include <limits>
#include <string>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkShiftScaleImageFilter.h"
#include "itkSigmoidImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include <itkAdaptiveHistogramEqualizationImageFilter.h>
#include "itkBinaryThresholdImageFilter.h"

const unsigned int Dimension = 2;
using PixelType = uint16_t;
using ImageType = itk::Image<PixelType, Dimension>;

int main( int argc, char * argv[] )
{
    if( argc < 6 ) {
        std::cerr << "Usage: " << std::endl;
        std::cerr << argv[0]
            << " inputImageFile outputImageFile alpha beta threshval"
            << std::endl;
        return EXIT_FAILURE;
    }

    const auto inputFilename = std::string(argv[1]);
    const auto outputFilename = std::string(argv[2]);
    const auto alpha = std::stod(argv[3]);
    const auto beta = std::stod(argv[4]);
    const auto threshPerc = std::stod(argv[5]);

    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(inputFilename);

    //auto rescaler = itk::RescaleIntensityImageFilter<ImageType, ImageType>::New();
    //rescaler->SetInput(reader->GetOutput());

    // Pass through sigmoid filter
    auto sigmoidFilter = itk::SigmoidImageFilter<ImageType, ImageType>::New();
    sigmoidFilter->SetInput(reader->GetOutput());
    sigmoidFilter->SetAlpha(alpha);
    sigmoidFilter->SetBeta(beta);

    // Binary threshold
    auto pixelMin = std::numeric_limits<PixelType>::min();
    auto pixelMax = std::numeric_limits<PixelType>::max();
    auto thresholdFilter = itk::BinaryThresholdImageFilter<ImageType, ImageType>::New();
    thresholdFilter->SetInput(sigmoidFilter->GetOutput());
    thresholdFilter->SetOutsideValue(pixelMin);
    thresholdFilter->SetInsideValue(pixelMax);
    const PixelType threshVal = PixelType( floor(threshPerc * pixelMax) );
    std::cout << "threshVal = " << uint32_t(threshVal) << std::endl;
    thresholdFilter->SetLowerThreshold(threshVal);
    thresholdFilter->SetUpperThreshold(pixelMax);

    // Adaptive Histogram Equalization
    /*
    auto equalizer = itk::AdaptiveHistogramEqualizationImageFilter<ImageType>::New();
    equalizer->SetInput(reader->GetOutput());
    equalizer->SetAlpha(alpha);
    equalizer->SetAlpha(beta);
    equalizer->SetRadius(radius);
    */

    // Rescale the intensities
    /*
    auto rescaler = itk::RescaleIntensityImageFilter<ImageType, ImageType>::New();
    auto maxValue = std::numeric_limits<PixelType>::max();
    rescaler->SetInput(equalizer->GetOutput());
    //rescaler->SetOutputMinimum(0);
    //rescaler->SetOutputMaximum(maxValue);

    auto shiftScaleFilter = itk::ShiftScaleImageFilter<ImageType, ImageType>::New();
    auto maxValue = std::numeric_limits<PixelType>::max();
    auto windowMinimum = PixelType( floor(0.02 * maxValue) );
    auto windowMaximum = PixelType( floor(0.99 * maxValue) );
    auto outputMinimum = 0;
    auto outputMaximum = maxValue;

    // Formula from Convert3D source
    // http://sourceforge.net/p/c3d/git/ci/master/tree/ConvertImageND.cxx
    // Ctrl-F for "stretch", look at the ShiftScaleImageFilter implementation
    auto a = double(outputMaximum - outputMinimum) / double(windowMaximum - windowMinimum);
    auto b = outputMinimum - windowMinimum * a;
    std::cout << "a: " << a << std::endl;
    std::cout << "b: " << b << std::endl;
    shiftScaleFilter->SetInput(reader->GetOutput());
    shiftScaleFilter->SetScale(a);
    shiftScaleFilter->SetShift(b / a);
    */

    auto writer = itk::ImageFileWriter<ImageType>::New();
    writer->SetInput(thresholdFilter->GetOutput());
    writer->SetFileName(outputFilename);

    try {
        writer->Update();
    } catch( itk::ExceptionObject & excep ) {
        std::cerr << "Exception catched !" << std::endl;
        std::cerr << excep << std::endl;
    }

    return EXIT_SUCCESS;
}
