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

// Need to be able to cast images from floats in their intermediate
// representation back to appropriate bit depths
#include "itkCastImageFilter.h"

// For the observer class
#include "itkCommand.h"


// Only working with volumes (3D)
const unsigned int Dimension = 3;

// Some typedefs to make things easier
typedef float PixelType;
typedef itk::Image<PixelType, Dimension> FixedImageType;
typedef itk::Image<PixelType, Dimension> MovingImageType;
typedef itk::VersorRigid3DTransform<double> TransformType;
typedef itk::RegularStepGradientDescentOptimizerv4<double> OptimizerType;
typedef itk::MeanSquaresImageToImageMetricv4<FixedImageType, MovingImageType> MetricType;
typedef itk::ImageRegistrationMethodv4<FixedImageType, MovingImageType, TransformType> RegistrationType;
typedef itk::ImageFileReader<FixedImageType> FixedImageReaderType;
typedef itk::ImageFileReader<MovingImageType> MovingImageReaderType;
typedef itk::CenteredTransformInitializer<TransformType, FixedImageType, MovingImageType> TransformInitializerType;
typedef TransformType::VersorType VersorType;
typedef VersorType::VectorType VectorType;
typedef OptimizerType::ScalesType OptimizerScalesType;
typedef itk::ResampleImageFilter<MovingImageType, FixedImageType> ResampleFilterType;
typedef uint16_t OutputPixelType;
typedef itk::Image<OutputPixelType, Dimension> OutputImageType;
typedef itk::CastImageFilter<FixedImageType, OutputImageType> CastFilterType;
typedef itk::ImageFileWriter<OutputImageType> WriterType;

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

void printHelp(char* programName);
