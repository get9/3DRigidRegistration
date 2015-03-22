#include "ConvertBitDepth.h"

int main(int argc, char **argv)
{
    if (argc < 3) {
        std::cerr << "Usage:" << std::endl;
        std::cerr << "    " << argv[0] << " dim infile outfile" << std::endl;
        return EXIT_FAILURE;
    }
    
    // XXX Add some error handling later for making sure images with the right
    // bit depth are passed in

    std::string infile = std::string(argv[1]);
    std::string outfile = std::string(argv[2]);

    bool didConvert = convertBitDepth< uint8_t, uint16_t >(infile, outfile);
    if (didConvert == false) {
        std::cerr << "[error]: did not convert to 16 bit" << std::endl;
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}
