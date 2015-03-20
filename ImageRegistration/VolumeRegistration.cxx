// Everything else for this application
#include "VolumeRegistration.h"


int main( int argc, char *argv[] )
{
    if( argc < 4 ) {
        printHelp(argv[0]);
        return EXIT_FAILURE;
    }

    // Instantiate Transform, Optimizer, Metric, and Registration objects
    MetricType::Pointer metric = MetricType::New();
    OptimizerType::Pointer optimizer = OptimizerType::New();
    RegistrationType::Pointer registration = RegistrationType::New();
    registration->SetMetric(metric);
    registration->SetOptimizer(optimizer);
    TransformType::Pointer initialTransform = TransformType::New();

    // Read in images, set them to fixed and moving images respectively
    FixedImageReaderType::Pointer fixedImageReader = FixedImageReaderType::New();
    MovingImageReaderType::Pointer movingImageReader = MovingImageReaderType::New();
    fixedImageReader->SetFileName(argv[1]);
    movingImageReader->SetFileName(argv[2]);
    registration->SetFixedImage(fixedImageReader->GetOutput());
    registration->SetMovingImage(movingImageReader->GetOutput());

    // Sets the initializer and initializes it
    TransformInitializerType::Pointer initializer = TransformInitializerType::New();
    initializer->SetTransform(initialTransform);
    initializer->SetFixedImage(fixedImageReader->GetOutput());
    initializer->SetMovingImage(movingImageReader->GetOutput());
    // Uses geometric centers. Other option is MomentsOn(), but that takes
    // forever.
    initializer->GeometryOn();
    initializer->InitializeTransform();

    initialTransform->SetIdentity();

    // Initialize the transform parameters
    VersorType rotation;
    VectorType axis;
    axis[0] = 0.0;
    axis[1] = 0.0;
    axis[2] = 1.0;
    const double angle = 0;
    rotation.Set(axis, angle);
    initialTransform->SetRotation(rotation);

    registration->SetInitialTransform(initialTransform);

    // Configure the Optimizer
    OptimizerScalesType optimizerScales(initialTransform->GetNumberOfParameters());
    const double translationScale = 1.0 / 1000.0;
    optimizerScales[0] = 1.0;
    optimizerScales[1] = 1.0;
    optimizerScales[2] = 1.0;
    optimizerScales[3] = translationScale;
    optimizerScales[4] = translationScale;
    optimizerScales[5] = translationScale;
    optimizer->SetScales(optimizerScales);
    optimizer->SetNumberOfIterations(200);
    optimizer->SetLearningRate(0.2);
    // "At what point do we not care about continuing registration?" (in mm)
    // Otherwise known as stop threshold
    optimizer->SetMinimumStepLength(0.001);
    // While optimizer is testing points, it may have found a good point, but
    // then tests another one. The last value is not necessarily the best one.
    // If we found another value, keep the best one.
    optimizer->SetReturnBestParametersAndValue(true);
    // Create the command observer and hook it to the optimizer
    CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
    optimizer->AddObserver(itk::IterationEvent(), observer);

    // Set up level registration (just use 1 level right now)
    const unsigned int numberOfLevels = 1;
    RegistrationType::ShrinkFactorsArrayType shrinkFactorsPerLevel;
    shrinkFactorsPerLevel.SetSize(1);
    shrinkFactorsPerLevel[0] = 1;
    // Need to smooth the image before you subsample it
    RegistrationType::SmoothingSigmasArrayType smoothingSigmasPerLevel;
    smoothingSigmasPerLevel.SetSize(1);
    smoothingSigmasPerLevel[0] = 0;
    // Parameters of the pyramid scheme
    registration->SetNumberOfLevels(numberOfLevels);
    registration->SetSmoothingSigmasPerLevel(smoothingSigmasPerLevel);
    registration->SetShrinkFactorsPerLevel(shrinkFactorsPerLevel);

    // Run the registration
    try {
        // Run the registration
        registration->Update();
        std::cout << "Optimizer stop condition: "
                  << registration->GetOptimizer()->GetStopConditionDescription()
                  << std::endl;
    }
    catch(itk::ExceptionObject& err) {
        std::cerr << "ExceptionObject caught !" << std::endl;
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }

    // Print out final parameters
    const TransformType::ParametersType finalParameters = registration->GetOutput()->Get()->GetParameters();
    std::cout << std::endl << std::endl;
    std::cout << "Result = " << std::endl;
    std::cout << " versor X      = " << finalParameters[0] << std::endl;
    std::cout << " versor Y      = " << finalParameters[1] << std::endl;
    std::cout << " versor Z      = " << finalParameters[2] << std::endl;
    std::cout << " Translation X = " << finalParameters[3]  << std::endl;
    std::cout << " Translation Y = " << finalParameters[4]  << std::endl;
    std::cout << " Translation Z = " << finalParameters[5]  << std::endl;
    std::cout << " Iterations    = " << optimizer->GetCurrentIteration() << std::endl;
    std::cout << " Metric value  = " << optimizer->GetValue() << std::endl;
    std::cout << std::endl;

    // Print out transformation matrix
    TransformType::Pointer finalTransform = TransformType::New();
    finalTransform->SetFixedParameters(registration->GetOutput()->Get()->GetFixedParameters());
    finalTransform->SetParameters(finalParameters);
    std::cout << "Matrix = " << std::endl << finalTransform->GetMatrix() << std::endl;
    std::cout << "Offset = " << std::endl << finalTransform->GetOffset() << std::endl;

    // Write the registered image out to a file so we can look at it and visually compare
    // 1. Apply transform to the fixed image to make it look like the moving image
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

    return EXIT_SUCCESS;
}

void printHelp(char* programName)
{
    std::cerr << "Missing Parameters " << std::endl;
    std::cerr << "Usage:" << std::endl;
    std::cerr << programName << " fixedImageFile movingImageFile outputImageFile" << std::endl;
}
