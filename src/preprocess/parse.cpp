#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <pugixml.hpp>

#include "parse.h"

namespace MCA2 {

// constructor
Parser::Parser(std::string &cfgFilePath) {
    std::unordered_map<std::string, std::string> config;
    parseConfigFile(cfgFilePath, config);

    taskInfo = TaskInfo(config["Calibration_xml"], config["RawImage_Path"], config["Output_Path"],
                        std::stoi(config["start_frame"]), std::stoi(config["end_frame"]),
                        std::stoi(config["height"]), std::stoi(config["width"]));

    
}

// private functions
int Parser::parseConfigFile(std::string &configFilePath,
                            std::unordered_map<std::string, std::string> &config) {
    std::ifstream configFile(configFilePath);

    if (!configFile.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(configFile, line)) {
        std::istringstream iss(line);
        std::string key, value;

        if (iss >> key >> value) {
            config[key] = value;
        }
    }
    configFile.close();

    // for (const auto &kv : config) {
    //     std::cout << kv.first << ": " << kv.second << std::endl;
    // }

    return 0;
}

int Parser::parseCalibXMLFile(std::string &calibXMLFilePath) {
    // implement
    return 0;
}

} // namespace MCA2