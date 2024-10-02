#include <iostream>
#include <opencv2/core.hpp>
#include <string>

#include "parse.hpp"
#include "preprocess.hpp"

// todo: 优化，boys序列通过扩图与garden等保持一致
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

    // preprocess
    MCA2::PreProcessor PreProcessor(seqInfo.diameter);
    PreProcessor.preprocess(seqInfo, taskInfo);

    return 0;
}
