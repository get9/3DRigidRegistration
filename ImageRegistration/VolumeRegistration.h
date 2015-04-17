// Registration stuff
#include "itkImageRegistrationMethodv4.h"
#include "itkMeanSquaresImageToImageMetricv4.h"
#include "itkSimilarity3DTransform.h"
#include "itkRegularStepGradientDescentOptimizerv4.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkCenteredTransformInitializer.h"

// Needed for I/O
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkResampleImageFilter.h"

// Need to be able to cast images from floats in their intermediate
// representation back to appropriate bit depths
#include "itkCastImageFilter.h"

// For the observer class
#include "itkCommand.h"

// Only working with 3D data
const auto TDimension = 3;

// Pixel types. Externally, they are of type TPixel. Internally, they are of type TInternalPixel
using TPixel         = float;
using TInternalPixel = float;

// Image types
using TFixedImage    = itk::Image<TPixel, TDimension>;
using TMovingImage   = itk::Image<TPixel, TDimension>;

// Registration stuff
using TTransform            = itk::Similarity3DTransform<double>;
using TOptimizer            = itk::RegularStepGradientDescentOptimizerv4<double>;
using TRegistration         = itk::ImageRegistrationMethodv4<TFixedImage, TMovingImage, TTransform>;
using TMetric               = itk::MeanSquaresImageToImageMetricv4<TFixedImage, TMovingImage>;
using TTransformInitializer = itk::CenteredTransformInitializer<TTransform, TFixedImage, TMovingImage>;

// Readers
using TFixedReader  = itk::ImageFileReader<TFixedImage>;
using TMovingReader = itk::ImageFileReader<TMovingImage>;

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
