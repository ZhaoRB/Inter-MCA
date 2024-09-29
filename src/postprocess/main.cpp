#include <iostream>
#include <opencv2/core.hpp>
#include <string>

#include "parse.hpp"
#include "postprocess.hpp"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file_path>" << std::endl;
        return 1;
    }
    std::string configFilePath = argv[1];

    MCA2::SequenceInfo seqInfo;
    MCA2::TaskInfo taskInfo;

    // Parse the configuration file
    MCA2::Parser parser;
    parser.parseConfigFile(configFilePath, taskInfo);
    parser.parseCalibXMLFile(taskInfo.calibrationFilePath, seqInfo);

    // postprocess
    MCA2::PostProcessor postprocessor(seqInfo.diameter);
    postprocessor.postprocess(seqInfo, taskInfo);

    return 0;
}
