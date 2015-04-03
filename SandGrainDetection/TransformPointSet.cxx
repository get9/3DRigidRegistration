#include <iostream>
#include "PointSetUtil.h"
#include "itkTranslationTransform.h"


template <uint32_t TDimension>
void doApplyTransform(std::string infile, std::string outfile)
{
    // Helpful aliases
    using TPointSet = itk::PointSet<double, TDimension>;
    using TPoint = typename TPointSet::PointType;
    using TTransform = itk::TranslationTransform<typename TPoint::CoordRepType, TDimension>;

    // Set up transform
    auto transform = TTransform::New();
    typename TTransform::OutputVectorType displacement;
    displacement[0] = 1.0;
    displacement[1] = 0.0;
    transform->Translate(displacement);

    // Read PointSet from file
    auto points = readFromFile<double, TDimension>(infile);

    // Apply transform
    try {
        auto transformedPoints = applyTransform<double, TDimension, TTransform>(points, transform);
        writeToFile<double, TDimension>(outfile, transformedPoints);
    } catch (itk::ExceptionObject& err) {
        std::cerr << "[error]: itk::ExceptionObject caught" << std::endl;
        std::cerr << err << std::endl;
        exit(1);
    }
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
    if (dimension == 2) {
        doApplyTransform<2>(infile, outfile);
    } else {
        doApplyTransform<3>(infile, outfile);
    }
}
