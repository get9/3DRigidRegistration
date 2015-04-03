#include "ExtractSandGrainCentroids.h"


int main(int argc, char **argv)
{
    // Verify number of params and parse and validate args
    if (argc < 7) {
        std::cout << "Usage: " << std::endl;
        std::cout << argv[0]
                  << " inputImage outputImage H threshVal bitdepth dimension"
                  << std::endl; 
        exit(1);
    }

    // Parse/validate args
    const std::string inputFilename = std::string(argv[1]);
    const std::string outputFilename = std::string(argv[2]);
    const double H = std::stod(argv[3]);
    if (H < 0.0 || H > 1.0) {
        std::cerr << "[error]: H must be in range [0, 1]" << std::endl;
        return EXIT_FAILURE;
    }
    const double threshPerc = std::stod(argv[4]);
    if (threshPerc < 0.0 || threshPerc > 1.0) {
        std::cerr << "[error]: threshPerc must be in range [0, 1]" << std::endl;
        return EXIT_FAILURE;
    }
    const uint32_t bitdepth = std::stoi(argv[5]);
    if (bitdepth != 8 && bitdepth != 16) {
        std::cerr << "[error]: bitdepth must be one of {8, 16}" << std::endl;
        return EXIT_FAILURE;
    }
    const uint32_t dimension = std::stoi(argv[6]);
    if (dimension != 2 && dimension != 3) {
        std::cerr << "[error]: dimension must be one of {2, 3}" << std::endl;
        return EXIT_FAILURE;
    }

    // Run detector
    try {
        if (bitdepth == 8 && dimension == 2) {
            extractSandGrainCentroids<uint8_t, 2>(inputFilename, outputFilename, H, threshPerc);
        } else if (bitdepth == 16 && dimension == 2) {
            extractSandGrainCentroids<uint16_t, 2>(inputFilename, outputFilename, H, threshPerc);
        } else if (bitdepth == 8 && dimension == 3) {
            extractSandGrainCentroids<uint8_t, 3>(inputFilename, outputFilename, H, threshPerc);
        } else {
            extractSandGrainCentroids<uint16_t, 3>(inputFilename, outputFilename, H, threshPerc);
        }
    } catch (itk::ExceptionObject& err) {
        std::cerr << "itk::ExceptionObject caught" << std::endl;
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
