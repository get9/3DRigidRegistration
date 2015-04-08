#include <iostream>
#include "PointSetUtil.h"
#include "itkSimilarity3DTransform.h"
#include "itkTransformMeshFilter.h"


const uint32_t TDimension = 3;

inline const double deg2rad(const double deg) {
    return (deg * M_PI / 180.0);
}

struct TranslateVector {
    double x;
    double y;
    double z;
};

void doApplyTransform(std::string infile, std::string outfile, const double rotateDegree,
                      TranslateVector v, double scale)
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
    translate[0] = v.x;
    translate[1] = v.y;
    translate[2] = v.z;
    transform->Translate(translate);
    TVector axis;
    TVersor rotation;
    axis[0] = 0.0;
    axis[1] = 0.0;
    axis[2] = 1.0;
    const double angle = deg2rad(rotateDegree);
    rotation.Set(axis, angle);
    transform->SetRotation(rotation);
    transform->SetScale(scale);

    // Read PointSet from file
    auto points = readFromFile<double, TDimension>(infile);

    // Apply transform
    auto transformedPoints = applyTransform<double, TDimension, TTransform>(points, transform);

    // Write back to file
    writeToFile<double, TDimension>(outfile, transformedPoints);
}

int main(int argc, char** argv)
{
    if (argc < 8) {
        std::cerr << "Usage:" << std::endl;
        std::cerr << "    " << argv[0] << " infile outfile rot tx ty tz scale" << std::endl;
        exit(1);
    }

    // Parse/validate input args
    const auto infile     = std::string(argv[1]);
    const auto outfile    = std::string(argv[2]);
    const auto degrees    = std::stod(argv[3]);
    const auto translateX = std::stod(argv[4]);
    const auto translateY = std::stod(argv[5]);
    const auto translateZ = std::stod(argv[6]);
    const auto scale      = std::stod(argv[7]);

    TranslateVector v = { translateX, translateY, translateZ };

    doApplyTransform(infile, outfile, degrees, v, scale);
}
