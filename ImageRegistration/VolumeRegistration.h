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
using TPixel         = uint8_t;
using TInternalPixel = double;

// Image types
using TFixedImage    = itk::Image<TPixel, TDimension>;
using TMovingImage   = itk::Image<TPixel, TDimension>;
using TInternalImage = itk::Image<TInternalPixel, TDimension>;

// Casters (to go from external to internal representation)
using TFixedCastFilter  = itk::CastImageFilter<TFixedImage, TInternalImage>;
using TMovingCastFilter = itk::CastImageFilter<TMovingImage, TInternalImage>;

// Registration stuff
using TTransform            = itk::Similarity3DTransform<TInternalPixel>;
using TOptimizer            = itk::RegularStepGradientDescentOptimizerv4<TInternalPixel>;
using TRegistration         = itk::ImageRegistrationMethodv4<TInternalImage, TInternalImage, TTransform>;
using TMetric               = itk::MeanSquaresImageToImageMetricv4<TInternalImage, TInternalImage>;
using TTransformInitializer = itk::CenteredTransformInitializer<TTransform, TInternalImage, TInternalImage>;

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
        typedef itk::RegularStepGradientDescentOptimizerv4<TInternalPixel> OptimizerType;
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
