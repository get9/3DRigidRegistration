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
#include <itkHConcaveImageFilter.h>
#include <itkCannyEdgeDetectionImageFilter.h>
#include <itkCastImageFilter.h>

const unsigned int Dimension = 2;
using PixelType = uint8_t;
using TInternalPixel = double;
using ImageType = itk::Image<PixelType, Dimension>;
using TInternalImage = itk::Image<TInternalPixel, Dimension>;

int main( int argc, char * argv[] )
{
    if( argc < 6 ) {
        std::cerr << "Usage: " << std::endl;
        std::cerr << argv[0]
            << " inputImageFile outputImageFile alpha beta threshval hperc"
            << std::endl;
        return EXIT_FAILURE;
    }

    const auto inputFilename = std::string(argv[1]);
    const auto outputFilename = std::string(argv[2]);
    const auto alpha = std::stod(argv[3]);
    const auto beta = std::stod(argv[4]);
    const auto threshPerc = std::stod(argv[5]);
    const auto hPerc = std::stod(argv[6]);

    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(inputFilename);

    /*
     * Rescale intensities
    auto rescaler = itk::RescaleIntensityImageFilter<ImageType, ImageType>::New();
    rescaler->SetInput(reader->GetOutput());
     */

    /*
     * Sigmoid Filter
     */
    // Pass through sigmoid filter
    /*
    auto sigmoidFilter = itk::SigmoidImageFilter<ImageType, ImageType>::New();
    sigmoidFilter->SetInput(reader->GetOutput());
    sigmoidFilter->SetAlpha(alpha);
    sigmoidFilter->SetBeta(beta);
    sigmoidFilter->SetOutputMaximum(std::numeric_limits<PixelType>::max());
    sigmoidFilter->SetOutputMinimum(std::numeric_limits<PixelType>::min());
    */

    /*
     * Convolution
     */
    // Set up Annulus Operator as convolution kernel
    const auto pixelMax = std::numeric_limits<PixelType>::max();
    const auto pixelMin = std::numeric_limits<PixelType>::min();
    using OperatorType = itk::AnnulusOperator<PixelType, Dimension>;
    auto annulusOperator = OperatorType();
    annulusOperator.NormalizeOff();
    annulusOperator.SetInnerRadius(0.0);
    annulusOperator.SetThickness(2.0);
    annulusOperator.SetExteriorValue(0);
    annulusOperator.SetAnnulusValue(pixelMax);
    annulusOperator.SetInteriorValue(pixelMax);
    annulusOperator.CreateOperator();

    // Convolution
    using ConvolutionFilterType = itk::NeighborhoodOperatorImageFilter<ImageType, ImageType>;
    auto convolutionFilter = ConvolutionFilterType::New();
    convolutionFilter->SetInput(reader->GetOutput());
    convolutionFilter->SetOperator(annulusOperator);

    // Cast from PixelType to double
    auto toInternal = itk::CastImageFilter<ImageType, TInternalImage>::New();
    toInternal->SetInput(convolutionFilter->GetOutput());

    // Get non-maxima suppressed image
    auto nonMaximaFilter = itk::CannyEdgeDetectionImageFilter<TInternalImage, TInternalImage>::New();
    nonMaximaFilter->SetInput(toInternal->GetOutput());
    nonMaximaFilter->SetVariance(2.0);
    nonMaximaFilter->SetUpperThreshold(0);
    nonMaximaFilter->SetLowerThreshold(0);
    auto suppressedImage = nonMaximaFilter->GetNonMaximumSuppressionImage();

    // Run HConcave filter over it, parameterized over H
    /*
    auto concaveFilter = itk::HConcaveImageFilter<ImageType, ImageType>::New();
    concaveFilter->SetInput(convolutionFilter->GetOutput());
    concaveFilter->SetHeight( uint32_t( floor(hPerc * std::numeric_limits<PixelType>::max()) ) );
    */
    
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

    // Cast it back for writing
    auto fromInternal = itk::CastImageFilter<TInternalImage, ImageType>::New();
    fromInternal->SetInput(suppressedImage);

    auto writer = itk::ImageFileWriter<ImageType>::New();
    //writer->SetInput(convolutionFilter->GetOutput());
    writer->SetInput(fromInternal->GetOutput());
    writer->SetFileName(outputFilename);

    try {
        writer->Update();
    } catch( itk::ExceptionObject & excep ) {
        std::cerr << "Exception catched !" << std::endl;
        std::cerr << excep << std::endl;
    }

    return EXIT_SUCCESS;
}
