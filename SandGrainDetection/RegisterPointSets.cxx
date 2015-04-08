#include <iostream>
#include <fstream>
#include <cmath>

#include "itkAffineTransform.h"
#include "itkEuclideanDistancePointMetric.h"
#include "itkPointSetToPointSetRegistrationMethod.h"
#include "itkEuclideanDistancePointMetric.h"
#include "itkSimilarity3DTransform.h"
#include "itkLevenbergMarquardtOptimizer.h"
#include "PointSetUtil.h"


const inline double rad2deg(const double rad)
{
    return (rad * 180.0 / M_PI);
}

class CommandIterationUpdate : public itk::Command
{
    public:
        typedef  CommandIterationUpdate Self;
        typedef  itk::Command           Superclass;
        typedef itk::SmartPointer<Self> Pointer;
        itkNewMacro(Self);
    protected:
        CommandIterationUpdate() {};
    public:
        typedef itk::LevenbergMarquardtOptimizer OptimizerType;
        typedef const OptimizerType *            OptimizerPointer;
        void Execute(itk::Object *caller, const itk::EventObject & event) ITK_OVERRIDE
        {
            Execute( (const itk::Object *)caller, event);
        }
        void Execute(const itk::Object * object, const itk::EventObject & event) ITK_OVERRIDE
        {
            OptimizerPointer optimizer = static_cast< OptimizerPointer >( object );
            if( !itk::IterationEvent().CheckEvent(&event) ) {
              return;
            }
            std::cout << optimizer->GetCachedValue() << std::endl;
            std::cout << optimizer->GetCachedCurrentPosition();
            std::cout << std::endl << std::endl;
        }
};


int main(int argc, char * argv[] )
{
    if( argc < 3 ) {
        std::cerr << "Usage:" << std::endl;
        std::cerr << "    " << argv[0]
                  << " fixedPointsFile  movingPointsFile" << std::endl;
        exit(1);;
    }

    // Define types for input containers
    const uint32_t TDimension = 3;
    using TPointSet = itk::PointSet<double, TDimension>;
    using TPoint = typename TPointSet::PointType;

    // Read points from filenames
    const auto fixedFilename  = std::string(argv[1]);
    const auto movingFilename = std::string(argv[2]);
    auto fixed  = readFromFile<double, TDimension>(fixedFilename); 
    auto moving = readFromFile<double, TDimension>(movingFilename); 

    // Set up registration infrastructure
    auto metric       = itk::EuclideanDistancePointMetric<TPointSet, TPointSet>::New();
    using TTransform  = itk::Similarity3DTransform<double>;
    auto transform    = TTransform::New();
    auto optimizer    = itk::LevenbergMarquardtOptimizer::New();
    auto registration = itk::PointSetToPointSetRegistrationMethod<TPointSet, TPointSet>::New();

    // Scale the translation components of the Transform in the Optimizer
    using TOptimizer = itk::LevenbergMarquardtOptimizer;
    TOptimizer::ScalesType scales(transform->GetNumberOfParameters());
    const double translationScale = 1000.0;
    const double rotationScale    = 1.0;
    const double scaleScale       = 1.0;
    scales[0] = 1.0 / rotationScale;
    scales[1] = 1.0 / rotationScale;
    scales[2] = 1.0 / rotationScale;
    scales[3] = 1.0 / rotationScale;
    scales[4] = 1.0 / translationScale;
    scales[5] = 1.0 / translationScale;
    scales[6] = 1.0 / scaleScale;

    // Next we setup the convergence criteria, and other properties required
    // by the optimizer.
    const uint64_t numberOfIterations =  1000;
    const double   gradientTolerance  =  1e-7; // convergence criterion
    const double   valueTolerance     =  1e-7; // convergence criterion
    const double   epsilonFunction    =  1e-7; // convergence criterion
    optimizer->SetScales(scales);
    optimizer->SetNumberOfIterations(numberOfIterations);
    optimizer->SetValueTolerance(valueTolerance);
    optimizer->SetGradientTolerance(gradientTolerance);
    optimizer->SetEpsilonFunction(epsilonFunction);
    optimizer->SetUseCostFunctionGradient(false);

    // Start with identity transform (we probably won't know what transform to 
    // start with
    transform->SetIdentity();
    registration->SetInitialTransformParameters(transform->GetParameters());

    // Finally, connect all the components required for the registration
    registration->SetMetric(metric);
    registration->SetOptimizer(optimizer);
    registration->SetTransform(transform);
    registration->SetFixedPointSet(fixed);
    registration->SetMovingPointSet(moving);

    // Connect an observer
    //CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
    //optimizer->AddObserver(itk::IterationEvent(), observer);

    // Run registration
    try {
        registration->Update();
    } catch(itk::ExceptionObject& e) {
        std::cout << e << std::endl;
        return EXIT_FAILURE;
    }

    // Get average difference of optimizer positions for each point
    double sum = 0.0;
    auto position = optimizer->GetValue();
    for (auto i = 0; i < position.GetSize(); ++i) {
        sum += position.GetElement(i);
    }
    std::cout << "Average difference = " << sum / position.GetSize() << std::endl;
    std::cout << std::endl;

    // Print out final parameters
    auto finalParameters = registration->GetOutput()->Get()->GetParameters();
    std::cout << "Result = " << std::endl;
    std::cout << " versor X        = " << finalParameters[0] << std::endl;
    std::cout << " versor Y        = " << finalParameters[1] << std::endl;
    std::cout << " versor Z        = " << finalParameters[2] << std::endl;
    std::cout << " Translation X   = " << finalParameters[3]  << std::endl;
    std::cout << " Translation Y   = " << finalParameters[4]  << std::endl;
    std::cout << " Translation Z   = " << finalParameters[5]  << std::endl;
    std::cout << " Isotropic Scale = " << finalParameters[6]  << std::endl;
    //std::cout << " Metric value    = " << optimizer->GetValue() << std::endl;
    std::cout << std::endl;

    // Print out transformation matrix
    auto finalTransform = TTransform::New();
    finalTransform->SetFixedParameters(registration->GetOutput()->Get()->GetFixedParameters());
    finalTransform->SetParameters(finalParameters);
    std::cout << "Matrix = " << std::endl << finalTransform->GetMatrix() << std::endl;
    std::cout << "Angle  = " << rad2deg(finalTransform->GetVersor().GetAngle()) << std::endl;
    std::cout << "Offset = " << finalTransform->GetOffset() << std::endl;
    std::cout << "Scale  = " << finalTransform->GetScale() << std::endl;

    // Run through moving PointSet and map back to fixed PointSet, verify they're close
    //for (auto i = 0; i < moving->GetNumberOfPoints(); ++i) {
    //    auto transformedMovingPoint = transform->TransformPoint(moving->GetPoint(i));
    //    auto pointDifference = transformedMovingPoint - fixed->GetPoint(i);
    //    std::cout << pointDifference.GetSquaredNorm() << std::endl;
    //}

    return EXIT_SUCCESS;
}
