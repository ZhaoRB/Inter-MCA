#include <iostream>
#include <opencv2/core.hpp>
#include <string>

#include "parse.hpp"
#include "utils.hpp"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file_path>" << std::endl;
        return 1;
    }

    std::string configFilePath = argv[1];

    // Parse the configuration file
    MCA2::Parser parser(configFilePath);

    std::string drawInputPath =
        "/Users/riverzhao/Project/Codec/0_lvc_codec/Inter-MCA/data/raw/miniGarden.bmp";
    std::string drawOutputPath = "/Users/riverzhao/Project/Codec/0_lvc_codec/Inter-MCA/data/center/"
                                 "all_center_miniGarden_cpp.png";
    MCA2::drawCenters(parser.sequenceInfo, drawInputPath, drawOutputPath);

    // cropper

    // predictor

    return 0;
}
