#include <iostream>
#include "PointSetUtil.h"
#include "itkSimilarity3DTransform.h"
#include "itkTransformMeshFilter.h"


inline double deg2rad(double deg) {
    return (deg * M_PI / 180.0);
}

template <uint32_t TDimension>
void doApplyTransform(std::string infile, std::string outfile)
{
    // Helpful aliases
    using TPointSet = itk::PointSet<double, TDimension>;
    using TPoint = typename TPointSet::PointType;
    using TTransform = itk::Similarity3DTransform<double>;
    using TOutputVector = typename TTransform::OutputVectorType;
    using TVersor = typename TTransform::VersorType;
    using TVector = typename TTransform::VectorType;

    // Set up transform
    auto transform = TTransform::New();
    TOutputVector translate;
    translate[0] = 1.5;
    translate[1] = 1.5;
    transform->Translate(translate);
    TVector axis;
    TVersor rotation;
    axis[0] = 0.0;
    axis[1] = 0.0;
    axis[2] = 1.0;
    const double angle = deg2rad(30.0);
    rotation.Set(axis, angle);
    transform->SetRotation(rotation);

    // Read PointSet from file
    auto points = readFromFile<double, TDimension>(infile);

    // Apply transform
    auto transformedPoints = applyTransform<double, TDimension, TTransform>(points, transform);

    // Write back to file
    writeToFile<double, TDimension>(outfile, transformedPoints);
}

int main(int argc, char** argv)
{
    if (argc < 4) {
        std::cerr << "Usage:" << std::endl;
        std::cerr << "    " << argv[0] << " infile outfile dim" << std::endl;
        exit(1);
    }

    // Parse/validate input args
    const auto infile = std::string(argv[1]);
    const auto outfile = std::string(argv[2]);
    const uint32_t dimension = std::stoi(argv[3]);
    if (dimension != 2 && dimension != 3) {
        std::cerr << "[error]: dimension must be one of {2, 3}" << std::endl;
        return EXIT_FAILURE;
    }

    // Call proper function based on bitdepth/dimension
    /*
    if (dimension == 2) {
        doApplyTransform<2>(infile, outfile);
    } else {
        doApplyTransform<3>(infile, outfile);
    }
    */
    doApplyTransform<3>(infile, outfile);
}
