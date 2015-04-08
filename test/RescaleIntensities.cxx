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
#include <itkAnnulusOperator.h>
#include <itkNeighborhoodOperatorImageFilter.h>

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

    /*
     * Rescale intensities
     */
    auto rescaler = itk::RescaleIntensityImageFilter<ImageType, ImageType>::New();
    rescaler->SetInput(reader->GetOutput());

    /*
     * Sigmoid Filter
     */
    // Pass through sigmoid filter
    auto sigmoidFilter = itk::SigmoidImageFilter<ImageType, ImageType>::New();
    sigmoidFilter->SetInput(reader->GetOutput());
    sigmoidFilter->SetAlpha(alpha);
    sigmoidFilter->SetBeta(beta);

    /*
     * Convolution
     */
    // Set up Annulus Operator as convolution kernel
    using OperatorType = itk::AnnulusOperator<PixelType, Dimension>;
    auto annulusOperator = OperatorType();
    annulusOperator.NormalizeOff();
    annulusOperator.SetInnerRadius(0.0);
    annulusOperator.SetThickness(5.0);
    annulusOperator.SetExteriorValue(0);
    annulusOperator.SetAnnulusValue(1);
    annulusOperator.SetInteriorValue(0);
    annulusOperator.CreateOperator();

    // Convolution
    using ConvolutionFilterType = itk::NeighborhoodOperatorImageFilter<ImageType, ImageType>;
    auto convolutionFilter = ConvolutionFilterType::New();
    convolutionFilter->SetInput(sigmoidFilter->GetOutput());
    convolutionFilter->SetOperator(annulusOperator);
    
    /*
     * Binary thresholding
    const auto pixelMin = std::numeric_limits<PixelType>::min();
    const auto pixelMax = std::numeric_limits<PixelType>::max();
    const auto thresholdFilter = itk::BinaryThresholdImageFilter<ImageType, ImageType>::New();
    thresholdFilter->SetInput(sigmoidFilter->GetOutput());
    thresholdFilter->SetOutsideValue(pixelMin);
    thresholdFilter->SetInsideValue(pixelMax);
    const PixelType threshVal = PixelType( floor(threshPerc * pixelMax) );
    std::cout << "threshVal = " << uint32_t(threshVal) << std::endl;
    thresholdFilter->SetLowerThreshold(threshVal);
    thresholdFilter->SetUpperThreshold(pixelMax);
     */

    // Adaptive Histogram Equalization
    /*
    auto equalizer = itk::AdaptiveHistogramEqualizationImageFilter<ImageType>::New();
    equalizer->SetInput(reader->GetOutput());
    equalizer->SetAlpha(alpha);
    equalizer->SetAlpha(beta);
    equalizer->SetRadius(radius);
    */

    /*
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
    writer->SetInput(convolutionFilter->GetOutput());
    writer->SetFileName(outputFilename);

    try {
        writer->Update();
    } catch( itk::ExceptionObject & excep ) {
        std::cerr << "Exception catched !" << std::endl;
        std::cerr << excep << std::endl;
    }

    return EXIT_SUCCESS;
}
