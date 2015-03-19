// Registration stuff
#include "itkImageRegistrationMethodv4.h"
#include "itkMeanSquaresImageToImageMetricv4.h"
#include "itkVersorRigid3DTransform.h"
#include "itkCenteredTransformInitializer.h"
#include "itkRegularStepGradientDescentOptimizerv4.h"

// Needed for I/O
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkResampleImageFilter.h"

// Need to be able to cast images to floats in their intermediate representation
// XXX Might get rid of this later (?)
#include "itkCastImageFilter.h"

// Not sure if these are needed at the moment
#include "itkSubtractImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkExtractImageFilter.h"

// For the observer class
#include "itkCommand.h"

// The observer class that will print out intermediate info of the optimizer
class CommandIterationUpdate : public itk::Command
{
    public:
        typedef CommandIterationUpdate Self;
        typedef itk::Command Superclass;
        typedef itk::SmartPointer<Self> Pointer;
        itkNewMacro(Self);
    
    protected:
        CommandIterationUpdate() {};
    
    public:
        typedef itk::RegularStepGradientDescentOptimizerv4<double> OptimizerType;
        typedef const OptimizerType* OptimizerPointer;
        void Execute(itk::Object* caller, const itk::EventObject & event)
        {
            Execute((const itk::Object*) caller, event);
        }
        void Execute(const itk::Object * object, const itk::EventObject & event)
        {
            OptimizerPointer optimizer = static_cast<OptimizerPointer>(object);
            if(!itk::IterationEvent().CheckEvent(&event)) {
                return;
            }
            std::cout << optimizer->GetCurrentIteration() << "     ";
            std::cout << optimizer->GetValue() << "     ";
            std::cout << optimizer->GetCurrentPosition() << std::endl;
        }
};

int main( int argc, char *argv[] )
{
    if( argc < 3 ) {
        std::cerr << "Missing Parameters " << std::endl;
        std::cerr << "Usage:" << std::endl;
        std::cerr << argv[0] << " fixedImageFile movingImageFile" << std::endl;
        return EXIT_FAILURE;
    }
    const unsigned int Dimension = 3;
    // Needs to be 16 bits, was using float before
    typedef float PixelType;
    typedef itk::Image<PixelType, Dimension> FixedImageType;
    typedef itk::Image<PixelType, Dimension> MovingImageType;

    // Instantiate Transform, Optimizer, Metric, and Registration objects
    typedef itk::VersorRigid3DTransform<double> TransformType;
    typedef itk::RegularStepGradientDescentOptimizerv4<double> OptimizerType;
    typedef itk::MeanSquaresImageToImageMetricv4<FixedImageType, MovingImageType> MetricType;
    typedef itk::ImageRegistrationMethodv4<FixedImageType, MovingImageType, TransformType> RegistrationType;

    MetricType::Pointer metric = MetricType::New();
    OptimizerType::Pointer optimizer = OptimizerType::New();
    RegistrationType::Pointer registration = RegistrationType::New();

    registration->SetMetric(metric);
    registration->SetOptimizer(optimizer);
    TransformType::Pointer initialTransform = TransformType::New();

    // Read in images, set them to fixed and moving images respectively
    typedef itk::ImageFileReader<FixedImageType> FixedImageReaderType;
    typedef itk::ImageFileReader<MovingImageType> MovingImageReaderType;
    FixedImageReaderType::Pointer fixedImageReader = FixedImageReaderType::New();
    MovingImageReaderType::Pointer movingImageReader = MovingImageReaderType::New();
    fixedImageReader->SetFileName(argv[1]);
    movingImageReader->SetFileName(argv[2]);
    registration->SetFixedImage(fixedImageReader->GetOutput());
    registration->SetMovingImage(movingImageReader->GetOutput());

    // Sets the initializer and initializes it
    typedef itk::CenteredTransformInitializer<TransformType,
                                              FixedImageType,
                                              MovingImageType> TransformInitializerType;
    TransformInitializerType::Pointer initializer = TransformInitializerType::New();
    initializer->SetTransform(initialTransform);
    initializer->SetFixedImage(fixedImageReader->GetOutput());
    initializer->SetMovingImage(movingImageReader->GetOutput());
    // Uses geometric centers. Other option is MomentsOn(), but that takes
    // forever.
    initializer->GeometryOn();
    initializer->InitializeTransform();

    // Initialize the transform parameters
    typedef TransformType::VersorType VersorType;
    typedef VersorType::VectorType VectorType;
    VersorType rotation;
    VectorType axis;
    axis[0] = 0.0;
    axis[1] = 0.0;
    axis[2] = 1.0;
    const double angle = 0;
    rotation.Set(axis, angle);
    initialTransform->SetRotation(rotation);

    // Set the registration method to use the initial transform
    registration->SetInitialTransform( initialTransform );

    // Configure the Optimizer
    typedef OptimizerType::ScalesType OptimizerScalesType;
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
    catch( itk::ExceptionObject & err ) {
        std::cerr << "ExceptionObject caught !" << std::endl;
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }

    // Print out final parameters
    const TransformType::ParametersType finalParameters =
        registration->GetOutput()->Get()->GetParameters();

    const double versorX           = finalParameters[0];
    const double versorY           = finalParameters[1];
    const double versorZ           = finalParameters[2];
    const double finalTranslationX = finalParameters[3];
    const double finalTranslationY = finalParameters[4];
    const double finalTranslationZ = finalParameters[5];
    const unsigned int numberOfIterations = optimizer->GetCurrentIteration();
    const double bestValue = optimizer->GetValue();

    // Print out results
    //
    std::cout << std::endl << std::endl;
    std::cout << "Result = " << std::endl;
    std::cout << " versor X      = " << versorX << std::endl;
    std::cout << " versor Y      = " << versorY << std::endl;
    std::cout << " versor Z      = " << versorZ << std::endl;
    std::cout << " Translation X = " << finalTranslationX  << std::endl;
    std::cout << " Translation Y = " << finalTranslationY  << std::endl;
    std::cout << " Translation Z = " << finalTranslationZ  << std::endl;
    std::cout << " Iterations    = " << numberOfIterations << std::endl;
    std::cout << " Metric value  = " << bestValue << std::endl;

    return EXIT_SUCCESS;
}
