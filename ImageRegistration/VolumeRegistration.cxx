#include <cmath>

// Everything else for this application
#include "itkRegionOfInterestImageFilter.h"
#include "VolumeRegistration.h"


int main(int argc, char *argv[])
{
    if(argc < 3) {
        std::cerr << "Missing Parameters " << std::endl;
        std::cerr << "Usage: " << std::endl;
        std::cerr << "    " << argv[0]
                  << " fixedImageFile movingImageFile" << std::endl; 
        return EXIT_FAILURE;
    }
    
    auto initialTransform = TTransform::New();
    auto optimizer        = TOptimizer::New();
    auto registration     = TRegistration::New();
    auto metric           = TMetric::New();
    auto fixedReader      = TFixedReader::New();
    auto movingReader     = TMovingReader::New();

    // Parse arguments
    auto fixedFilename  = std::string(argv[1]);
    auto movingFilename = std::string(argv[2]);

    // Read in data
    fixedReader->SetFileName(fixedFilename);
    movingReader->SetFileName(movingFilename);

    // Get the size of the fixed image
    fixedReader->Update();
    movingReader->Update();
    auto fixedSize = fixedReader->GetOutput()->GetLargestPossibleRegion().GetSize();
    auto movingSize = movingReader->GetOutput()->GetLargestPossibleRegion().GetSize();

    // Extract a region of interest from the fixed reader in order to register to that
    /*
    auto roiExtractor = itk::RegionOfInterestImageFilter<TFixedImage, TFixedImage>::New();
    roiExtractor->SetInput(fixedReader->GetOutput());
    TFixedImage::IndexType start;
    start[0] = uint32_t( floor(fixedSize[0] * 0.45 - movingSize[0] / 2)) - 50;
    start[1] = uint32_t( floor(fixedSize[1] * 0.55 - movingSize[1] / 2)) - 50;
    start[2] = 50;
    TFixedImage::SizeType size;
    size[0] = movingSize[0] + 100;
    size[1] = movingSize[1] + 100;
    size[2] = movingSize[2] + 100;
    std::cout << "Fixed size   = " << fixedSize << std::endl;
    std::cout << "Moving size  = " << movingSize << std::endl;
    std::cout << "Region start = " << start << std::endl;
    std::cout << "Region size  = " << size << std::endl;
    TFixedImage::RegionType regionOfInterest(start, size);
    roiExtractor->SetRegionOfInterest(regionOfInterest);
    */

    /*
     * Set up initial first transformation - best guess
     */
    // Rotation
    TTransform::VectorType axis;
    axis[0] = 0.0;
    axis[1] = 0.0;
    axis[2] = 1.0;
    const double angle = 0.0;
    TTransform::VersorType rotation;
    rotation.Set(axis, angle);
    initialTransform->SetRotation(rotation);

    // Scaling
    initialTransform->SetScale(1.0);

    // Translation
    TTransform::TranslationType translate;
    translate[0] = uint32_t( floor(fixedSize[0] * 0.45) );
    translate[1] = uint32_t( floor(fixedSize[1] * 0.55) );
    translate[2] = 107;
    std::cout << "Translating " << translate << std::endl;
    initialTransform->SetTranslation(translate);

    // Set up initialTransform initializer
    auto initializer = TTransformInitializer::New();
    initializer->SetTransform(initialTransform);
    initializer->SetFixedImage(fixedReader->GetOutput());
    initializer->SetMovingImage(movingReader->GetOutput());
    initializer->GeometryOn();
    initializer->InitializeTransform();

    // Connect all components
    registration->SetOptimizer(optimizer);
    registration->SetMetric(metric);
    registration->SetInitialTransform(initialTransform);
    registration->SetFixedImage(fixedReader->GetOutput());
    registration->SetMovingImage(movingReader->GetOutput());

    // Configure the Optimizer
    TOptimizer::ScalesType optimizerScales(initialTransform->GetNumberOfParameters());
    const double translationScale = 1.0 / 1000.0;
    optimizerScales[0] = 1.0;
    optimizerScales[1] = 1.0;
    optimizerScales[2] = 1.0;
    optimizerScales[3] = translationScale;
    optimizerScales[4] = translationScale;
    optimizerScales[5] = translationScale;
    optimizerScales[6] = 1.0;
    optimizer->SetMinimumStepLength(0.001);
    optimizer->SetScales(optimizerScales);
    optimizer->SetNumberOfIterations(200);
    optimizer->SetLearningRate(0.2);
    optimizer->SetReturnBestParametersAndValue(true);

    CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
    optimizer->AddObserver(itk::IterationEvent(), observer);

    // Set levels of registration
    const auto numberOfLevels = 1;
    TRegistration::ShrinkFactorsArrayType shrinkFactorsPerLevel;
    shrinkFactorsPerLevel.SetSize(1);
    shrinkFactorsPerLevel[0] = 1;
    TRegistration::SmoothingSigmasArrayType smoothingSigmasPerLevel;
    smoothingSigmasPerLevel.SetSize(1);
    smoothingSigmasPerLevel[0] = 0;
    registration->SetNumberOfLevels(numberOfLevels);
    registration->SetSmoothingSigmasPerLevel(smoothingSigmasPerLevel);
    registration->SetShrinkFactorsPerLevel(shrinkFactorsPerLevel);

    try {
        registration->Update();
        std::cout << "Optimizer stop condition: "
                            << registration->GetOptimizer()->GetStopConditionDescription()
                            << std::endl;
    } catch(itk::ExceptionObject& err) {
        std::cout << "ExceptionObject caught!" << std::endl;
        std::cout << err << std::endl;
        return EXIT_FAILURE;
    }
    
    // Print out final parameters
    auto finalParameters = registration->GetOutput()->Get()->GetParameters();
    std::cout << std::endl << std::endl;
    std::cout << "Result = " << std::endl;
    std::cout << " versor X        = " << finalParameters[0] << std::endl;
    std::cout << " versor Y        = " << finalParameters[1] << std::endl;
    std::cout << " versor Z        = " << finalParameters[2] << std::endl;
    std::cout << " Translation X   = " << finalParameters[3]  << std::endl;
    std::cout << " Translation Y   = " << finalParameters[4]  << std::endl;
    std::cout << " Translation Z   = " << finalParameters[5]  << std::endl;
    std::cout << " Isotropic Scale = " << finalParameters[6]  << std::endl;
    std::cout << " Iterations      = " << optimizer->GetCurrentIteration() << std::endl;
    std::cout << " Metric value    = " << optimizer->GetValue() << std::endl;
    std::cout << std::endl;

    // Print out transformation matrix
    auto finalTransform = TTransform::New();
    finalTransform->SetFixedParameters(registration->GetOutput()->Get()->GetFixedParameters());
    finalTransform->SetParameters(finalParameters);
    std::cout << "Matrix = " << std::endl << finalTransform->GetMatrix() << std::endl;
    std::cout << "Offset = " << std::endl << finalTransform->GetOffset() << std::endl;

    // Write the registered image out to a file so we can look at it and visually compare
    // 1. Apply initialTransform to the fixed image to make it look like the moving image
    /*
    ResampleFilterType::Pointer resampler = ResampleFilterType::New();
    resampler->SetTransform(finalTransform);
    resampler->SetInput(movingImageReader->GetOutput());
    FixedImageType::Pointer fixedImage = fixedImageReader->GetOutput();
    resampler->SetSize(fixedImage->GetLargestPossibleRegion().GetSize());
    resampler->SetOutputOrigin(fixedImage->GetOrigin());
    resampler->SetOutputSpacing(fixedImage->GetSpacing());
    resampler->SetOutputDirection(fixedImage->GetDirection());
    resampler->SetDefaultPixelValue(100);
    // 2. Cast the image from float values to 16-bit values
    WriterType::Pointer writer = WriterType::New();
    CastFilterType::Pointer caster = CastFilterType::New();
    writer->SetFileName(argv[3]);
    caster->SetInput(resampler->GetOutput());
    writer->SetInput(caster->GetOutput());

    // 3. Write the registered image to file
    try {
        writer->Update();
    }
    catch (itk::ExceptionObject& err) {
        std::cerr << "ExceptionObject caught!" << std::endl;
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }
    */

    return EXIT_SUCCESS;
}
