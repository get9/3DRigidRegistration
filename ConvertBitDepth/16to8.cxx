#include "ConvertBitDepth.h"

int main(int argc, char **argv)
{
    if (argc < 3) {
        std::cerr << "Usage:" << std::endl;
        std::cerr << "    " << argv[0] << " infile outfile" << std::endl;
        return EXIT_FAILURE;
    }
    
    // XXX Add some error handling later for making sure images with the right
    // bit depth are passed in

    const std::string infile = std::string(argv[1]);
    const std::string outfile = std::string(argv[2]);

    const bool didConvert = convertBitDepth< uint16_t, uint8_t >(infile, outfile);
    if (didConvert == false) {
        std::cerr << "[error]: did not convert to 8 bit" << std::endl;
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}
