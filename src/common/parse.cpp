#include <cmath>
#include <fstream>
#include <iostream>
#include <opencv2/core/types.hpp>
#include <ostream>
#include <pugixml.hpp>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

#include "data-structure.hpp"
#include "parse.hpp"
#include "utils.hpp"

namespace MCA2 {

int Parser::parseConfigFile(std::string &configFilePath, TaskInfo &taskInfo) {
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

    if (!hasFormatSpecifier(config["RawImage_Path"])) {
        // std::cout << "Warning: RawImage_Path don't have a format specifier" << std::endl;
        config["end_frame"] = config["start_frame"];
    }

    taskInfo = TaskInfo(config["Calibration_xml"], config["RawImage_Path"], config["Output_Path"],
                        std::stoi(config["start_frame"]), std::stoi(config["end_frame"]));
    configFile.close();

    return 0;
}

void Parser::setRowAndColNums(SequenceInfo &seqInfo) {
    seqInfo.colNum =
        std::round((seqInfo.rtop.x - seqInfo.ltop.x) / (seqInfo.diameter / 2 * sqrt(3))) + 1;

    if (seqInfo.rtop.x + seqInfo.diameter / 2 * sqrt(3) < seqInfo.width) {
        seqInfo.colNum = seqInfo.colNum + 1;
    }

    seqInfo.rowNum = std::round((seqInfo.lbot.y - seqInfo.ltop.y) / seqInfo.diameter) + 1;
}

int Parser::parseCalibXMLFile(std::string &calibXMLFilePath, SequenceInfo &seqInfo) {
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

    seqInfo.diameter = root.child("diameter").text().as_double();
    seqInfo.direction = root.child("direction").text().as_int();
    seqInfo.width = root.child("width").text().as_int();
    seqInfo.height = root.child("height").text().as_int();

    const pugi::xml_node centerNode = root.child("centers");
    seqInfo.ltop = cv::Point2d(centerNode.child("ltop").child("x").text().as_double(),
                               centerNode.child("ltop").child("y").text().as_double());
    seqInfo.rtop = cv::Point2d(centerNode.child("rtop").child("x").text().as_double(),
                               centerNode.child("rtop").child("y").text().as_double());
    seqInfo.lbot = cv::Point2d(centerNode.child("lbot").child("x").text().as_double(),
                               centerNode.child("lbot").child("y").text().as_double());
    seqInfo.rbot = cv::Point2d(centerNode.child("rbot").child("x").text().as_double(),
                               centerNode.child("rbot").child("y").text().as_double());

    setRowAndColNums(seqInfo);

    // transpose (Raytrix R8 R32)
    if (seqInfo.direction == 1) {
        std::swap(seqInfo.rtop, seqInfo.lbot);
        std::swap(seqInfo.colNum, seqInfo.rowNum);
    }

    calAllCenterPoints(seqInfo);

    return 0;
}

void Parser::calAllCenterPoints(SequenceInfo &seqInfo) {

    seqInfo.centers.resize(seqInfo.rowNum * seqInfo.colNum);

    // if column num is odd number, rtop and rbot are in the last column
    int gapColNum = seqInfo.colNum % 2 == 1 ? seqInfo.colNum - 1 : seqInfo.colNum - 2;
    cv::Point2d disCol = (seqInfo.rtop - seqInfo.ltop) / gapColNum;

    int idx = 0;
    for (int col = 0; col < seqInfo.colNum; ++col) {
        double ratioL = (static_cast<double>(seqInfo.colNum) - 1 - col) /
                        (static_cast<double>(seqInfo.colNum) - 1);
        double ratioR = col / (static_cast<double>(seqInfo.colNum) - 1);

        cv::Point2d disRow =
            (ratioL * (seqInfo.lbot - seqInfo.ltop) + ratioR * (seqInfo.rbot - seqInfo.rtop)) /
            (seqInfo.rowNum - 1);

        cv::Point2d firstPointOfColumn = seqInfo.ltop + disCol * col;
        if (col % 2 == 1) {
            firstPointOfColumn = firstPointOfColumn + disRow / 2;
        }

        seqInfo.centers[idx++] = firstPointOfColumn;

        for (int row = 1; row < seqInfo.rowNum; ++row) {
            seqInfo.centers[idx++] = firstPointOfColumn + row * disRow;
        }
    }
}

} // namespace MCA2