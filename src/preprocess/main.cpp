#include <iostream>
#include <opencv2/core.hpp>
#include <string>

#include "parse.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file_path>" << std::endl;
        return 1;
    }

    std::string configFilePath = argv[1];

    // Parse the configuration file
    MCA2::Parser parser(configFilePath);

    // cropper

    // predictor

    return 0;
}
