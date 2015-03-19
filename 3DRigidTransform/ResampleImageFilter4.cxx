#include <string>
#include <cmath>
#include <cstdlib>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkResampleImageFilter.h"
#include "itkVersorRigid3DTransform.h"

inline double deg2rad(double angle)
{
    return angle * (M_PI / 180.0);
}

int main( int argc, char * argv[] )
{
    if( argc < 4 ) {
        std::cerr << "Usage: " << std::endl;
        std::cerr << argv[0] << " input output rot" << std::endl;
        return EXIT_FAILURE;
    }

    const   unsigned int  Dimension = 3;
    typedef unsigned char InputPixelType;
    typedef unsigned char OutputPixelType;
    typedef double        InternalPixeltype;

    typedef itk::Image<InputPixelType,  Dimension> InputImageType;
    typedef itk::Image<OutputPixelType, Dimension> OutputImageType;

    // Set up reader/writer
    typedef itk::ImageFileReader<InputImageType> ReaderType;
    typedef itk::ImageFileWriter<OutputImageType> WriterType;
    ReaderType::Pointer reader = ReaderType::New();
    WriterType::Pointer writer = WriterType::New();
    reader->SetFileName( argv[1] );
    writer->SetFileName( argv[2] );

    const double angleInDegrees = atof( argv[3] );

    typedef itk::ResampleImageFilter<InputImageType, OutputImageType>    FilterType;
    FilterType::Pointer filter = FilterType::New();

    typedef itk::VersorRigid3DTransform<double> TransformType;
    TransformType::Pointer transform = TransformType::New();

    typedef itk::LinearInterpolateImageFunction<InputImageType, double> InterpolatorType;
    InterpolatorType::Pointer interpolator = InterpolatorType::New();
    filter->SetInterpolator(interpolator);
    filter->SetDefaultPixelValue(0);

    reader->Update();

    const InputImageType* inputImage = reader->GetOutput();
    const InputImageType::SpacingType& spacing = inputImage->GetSpacing();
    const InputImageType::PointType& origin    = inputImage->GetOrigin();
    InputImageType::SizeType size = inputImage->GetLargestPossibleRegion().GetSize();

    // Configure filter
    filter->SetOutputOrigin( origin );
    filter->SetOutputSpacing( spacing );
    filter->SetOutputDirection( inputImage->GetDirection() );
    filter->SetSize( size );
    filter->SetInput( reader->GetOutput() );
    writer->SetInput( filter->GetOutput() );

    //    Rotations are performed around the origin of physical coordinates---not
    //    the image origin nor the image center. Hence, the process of
    //    positioning the output image frame as it is shown in Figure
    //    \ref{fig:ResampleImageFilterOutput10} requires three steps.    First, the
    //    image origin must be moved to the origin of the coordinate system. This
    //    is done by applying a translation equal to the negative values of the
    //    image origin.
    //
    TransformType::OutputVectorType translation1;
    const double imageCenterX = origin[0] + spacing[0] * size[0] / 2.0;
    const double imageCenterY = origin[1] + spacing[1] * size[1] / 2.0;
    const double imageCenterZ = origin[2] + spacing[2] * size[2] / 2.0;
    translation1[0] = -imageCenterX;
    translation1[1] = -imageCenterY;
    translation1[2] = -imageCenterZ;
    transform->Translate( translation1 );

    std::cout << "imageCenterX = " << imageCenterX << std::endl;
    std::cout << "imageCenterY = " << imageCenterY << std::endl;
    std::cout << "imageCenterZ = " << imageCenterZ << std::endl;

    //    In a second step, the rotation is specified using the method
    // Compute the matrix directly and set it to generate the Versor for us.
    // Rotote along the Z-axis
    TransformType::AxisType axes;
    axes[0] = 0.0;
    axes[1] = 0.0;
    axes[2] = 1.0;
    TransformType::VersorType rotation;
    rotation.Set(axes, deg2rad(atof(argv[3])));
    transform->SetRotation(rotation);

    //    The third and final step requires translating the image origin back to
    //    its previous location. This is be done by applying a translation equal
    //    to the origin values.
    TransformType::OutputVectorType translation2;
    translation2[0] =     imageCenterX;
    translation2[1] =     imageCenterY;
    translation2[2] =     imageCenterZ;
    transform->Translate( translation2, false );
    filter->SetTransform( transform );

    //    The output of the resampling filter is connected to a writer and the
    //    execution of the pipeline is triggered by a writer update.
    try {
        writer->Update();
    }
    catch( itk::ExceptionObject & excep ) {
        std::cerr << "Exception caught !" << std::endl;
        std::cerr << excep << std::endl;
    }

    return EXIT_SUCCESS;
}
