#include <iostream>
#include <fstream>

#include "itkAffineTransform.h"
#include "itkEuclideanDistancePointMetric.h"
#include "itkPointSetToPointSetRegistrationMethod.h"
#include "itkEuclideanDistancePointMetric.h"
#include "itkSimilarity3DTransform.h"
#include "itkLevenbergMarquardtOptimizer.h"
#include "PointSetUtil.h"


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
    using TContainer = typename TPointSet::PointsContainer;
    auto fixedPointsContainer = TContainer::New();
    auto movingPointsContainer = TContainer::New();

    // Read points from filenames
    auto fixedFilename = std::string(argv[1]);
    auto movingFilename = std::string(argv[2]);
    auto fixed = readFromFile<double, TDimension>(fixedFilename); 
    auto moving = readFromFile<double, TDimension>(movingFilename); 

    // Set up registration infrastructure
    auto metric = itk::EuclideanDistancePointMetric<TPointSet, TPointSet>::New();
    auto transform = itk::Similarity3DTransform<double>::New();
    using TOptimizer = itk::LevenbergMarquardtOptimizer;
    auto optimizer = TOptimizer::New();
    optimizer->SetUseCostFunctionGradient(false);
    auto registration = itk::PointSetToPointSetRegistrationMethod<TPointSet, TPointSet>::New();

    // Scale the translation components of the Transform in the Optimizer
    TOptimizer::ScalesType scales(transform->GetNumberOfParameters());
    const double translationScale = 1000.0;
    const double rotationScale = 1.0;
    const double scaleScale = 1.0;
    scales[0] = 1.0 / rotationScale;
    scales[1] = 1.0 / rotationScale;
    scales[2] = 1.0 / rotationScale;
    scales[3] = 1.0 / rotationScale;
    scales[4] = 1.0 / translationScale;
    scales[5] = 1.0 / translationScale;
    scales[6] = 1.0 / scaleScale;

    // Next we setup the convergence criteria, and other properties required
    // by the optimizer.
    uint64_t numberOfIterations =  1000;
    double   gradientTolerance  =  1e-7; // convergence criterion
    double   valueTolerance     =  1e-7; // convergence criterion
    double   epsilonFunction    =  1e-7; // convergence criterion
    optimizer->SetScales(scales);
    optimizer->SetNumberOfIterations(numberOfIterations);
    optimizer->SetValueTolerance(valueTolerance);
    optimizer->SetGradientTolerance(gradientTolerance);
    optimizer->SetEpsilonFunction(epsilonFunction);

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
    CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
    optimizer->AddObserver(itk::IterationEvent(), observer);

    // Run registration
    try {
        registration->Update();
    } catch(itk::ExceptionObject& e) {
        std::cout << e << std::endl;
        return EXIT_FAILURE;
    }

    // Run through moving PointSet and map back to fixed PointSet, verify they're close
    for (auto i = 0; i < moving->GetNumberOfPoints(); ++i) {
        std::cout << transform->TransformPoint(moving->GetPoint(i)) << std::endl;
    }

    return EXIT_SUCCESS;
}
