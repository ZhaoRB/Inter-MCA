#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
// #include <sstream>

#include "parse.h"

namespace MCA2 {
SequenceInfo::SequenceInfo() {}

TaskInfo::TaskInfo() {}

int Parser::parseConfigFile(std::string &configFilePath) {
    std::ifstream configFile(configFilePath);
    std::unordered_map<std::string, std::string> config;

    if (!configFile.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return 1;
    }

    std::string line;
    int count = 0;
    while (std::getline(configFile, line)) {
        std::cout << count++ << line << std::endl;
        // std::istringstream iss(line);
        // std::string key;
        // std::string value;

        // if (std::getline(iss, key, ' ') && std::getline(iss, value)) {
        //     config[key] = value;
        // }
    }

    configFile.close();

    return 0;
}

int Parser::parseCalibXMLFile(std::string &calibXMLFilePath) {
    // implement
    return 0;
}
} // namespace MCA2