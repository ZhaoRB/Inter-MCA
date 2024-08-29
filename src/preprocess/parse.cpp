#include <fstream>
#include <iostream>
#include <opencv2/core/types.hpp>
#include <ostream>
#include <pugixml.hpp>
#include <sstream>
#include <string>
#include <unordered_map>

#include "parse.hpp"

namespace MCA2 {

// constructor
Parser::Parser(std::string &cfgFilePath) {
    parseConfigFile(cfgFilePath);
    parseCalibXMLFile(taskInfo.calibrationFilePath);
}

// private functions
int Parser::parseConfigFile(std::string &configFilePath) {
    std::ifstream configFile(configFilePath);
    std::unordered_map<std::string, std::string> config;

    if (!configFile.is_open()) {
        std::cerr << "Fail to load config file" << std::endl;
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

    taskInfo = TaskInfo(config["Calibration_xml"], config["RawImage_Path"], config["Output_Path"],
                        std::stoi(config["start_frame"]), std::stoi(config["end_frame"]),
                        std::stoi(config["height"]), std::stoi(config["width"]));
    configFile.close();

    return 0;
}

int Parser::parseCalibXMLFile(std::string &calibXMLFilePath) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(calibXMLFilePath.c_str());

    if (!result) {
        std::cerr << "Fail to load XML file: " << result.description() << std::endl;
        return 1;
    }

    const pugi::xml_node root = doc.child("TSPCCalibData");
    if (root.empty()) {
        std::cerr << "Missing xml node `TSPCCalibData` when initializing" << std::endl;
    }

    sequenceInfo.diameter = root.child("diameter").text().as_double();
    sequenceInfo.rotationAngle = root.child("rotation").text().as_double();

    const pugi::xml_node centerNode = root.child("centers");
    sequenceInfo.rowNum = centerNode.child("rows").text().as_int();
    sequenceInfo.colNum = centerNode.child("cols").text().as_int();

    sequenceInfo.ltop = cv::Point2d(centerNode.child("ltop").child("x").text().as_double(),
                                    centerNode.child("ltop").child("y").text().as_double());
    sequenceInfo.rtop = cv::Point2d(centerNode.child("rtop").child("x").text().as_double(),
                                    centerNode.child("rtop").child("y").text().as_double());
    sequenceInfo.lbot = cv::Point2d(centerNode.child("lbot").child("x").text().as_double(),
                                    centerNode.child("lbot").child("y").text().as_double());

    if (!centerNode.child("rbot").empty()) {
        sequenceInfo.isThreePoints = false;
        sequenceInfo.rbot = cv::Point2d(centerNode.child("rbot").child("x").text().as_double(),
                                        centerNode.child("rbot").child("y").text().as_double());
    }

    calAllCenterPoints();

    return 0;
}

void Parser::calAllCenterPoints() {
    double xBias = sequenceInfo.diameter / 2 * sqrt(3);
    double yBias = sequenceInfo.diameter / 2;
    cv::Point2d colGap, rowGap;
    cv::Point2d ltopEven(sequenceInfo.ltop.x + xBias, sequenceInfo.ltop.y + yBias);

    if (sequenceInfo.isThreePoints) {
        colGap = (sequenceInfo.rtop - ltopEven) / (sequenceInfo.colNum / 2 - 1);
        rowGap = (sequenceInfo.lbot - sequenceInfo.ltop) / (sequenceInfo.rowNum - 1);

    } else {
        // todo
    }

    sequenceInfo.centers.resize(sequenceInfo.colNum * sequenceInfo.rowNum);
    sequenceInfo.centers[0] = sequenceInfo.ltop;
    sequenceInfo.centers[1] = ltopEven;

    std::cout << "colGap: " << colGap.x << ", " << colGap.y << "\n"
              << "rowGap: " << rowGap.x << ", " << rowGap.y << std::endl;

    std::cout << sequenceInfo.centers.size() << std::endl;
    std::cout << "1: " << sequenceInfo.centers[0].x << ", " << sequenceInfo.centers[0].y << "\n"
              << "2: " << sequenceInfo.centers[1].x << ", " << sequenceInfo.centers[1].y
              << std::endl;

    for (int i = 2; i < sequenceInfo.colNum; i++) {
        sequenceInfo.centers[i] = sequenceInfo.centers[i - 2] + colGap;
    }
    for (int i = 1; i < sequenceInfo.rowNum; i++) {
        for (int j = 0; j < sequenceInfo.colNum; j++) {
            sequenceInfo.centers[i * sequenceInfo.colNum + j] =
                sequenceInfo.centers[(i - 1) * sequenceInfo.colNum + j] + rowGap;
        }
    }
}

} // namespace MCA2