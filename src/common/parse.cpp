#include <cmath>
#include <fstream>
#include <iostream>
#include <opencv2/core/types.hpp>
#include <ostream>
#include <pugixml.hpp>
#include <sstream>
#include <string>
#include <unordered_map>

#include "data-structure.hpp"
#include "parse.hpp"

namespace MCA2 {

// private functions
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

    // todo: height width 是否保留？
    taskInfo = TaskInfo(config["Calibration_xml"], config["RawImage_Path"], config["Output_Path"],
                        std::stoi(config["start_frame"]), std::stoi(config["end_frame"]),
                        std::stoi(config["height"]), std::stoi(config["width"]));
    configFile.close();

    return 0;
}

void Parser::setRowAndColNums(SequenceInfo &seqInfo) {
    seqInfo.colNum =
        std::round((seqInfo.rtop.x - seqInfo.ltop.x) / (seqInfo.diameter / 2 * sqrt(3))) + 1;
    seqInfo.rowNum = std::round((seqInfo.lbot.y - seqInfo.rbot.y) / seqInfo.diameter) + 1;
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

    const pugi::xml_node centerNode = root.child("centers");
    seqInfo.rowNum = centerNode.child("rows").text().as_int();
    seqInfo.colNum = centerNode.child("cols").text().as_int();

    seqInfo.ltop = cv::Point2d(centerNode.child("ltop").child("x").text().as_double(),
                               centerNode.child("ltop").child("y").text().as_double());
    seqInfo.rtop = cv::Point2d(centerNode.child("rtop").child("x").text().as_double(),
                               centerNode.child("rtop").child("y").text().as_double());
    seqInfo.lbot = cv::Point2d(centerNode.child("lbot").child("x").text().as_double(),
                               centerNode.child("lbot").child("y").text().as_double());
    seqInfo.rbot = cv::Point2d(centerNode.child("rbot").child("x").text().as_double(),
                               centerNode.child("rbot").child("y").text().as_double());

    setRowAndColNums(seqInfo);
    calAllCenterPoints(seqInfo);

    return 0;
}

void Parser::calAllCenterPoints(SequenceInfo &seqInfo) {
    // TSPC: NewMiniGarden, Motherboard, HandTools
    // colNum - 2, rtop 是倒数第二列
    seqInfo.centers.resize(seqInfo.rowNum * seqInfo.colNum);

    cv::Point2d disCol = seqInfo.rtop - seqInfo.ltop;

    // todo: 优化赋值过程
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
        seqInfo.centers[col] = firstPointOfColumn;

        for (int row = 1; row < seqInfo.rowNum; ++row) {
            seqInfo.centers[row * seqInfo.colNum + col] = firstPointOfColumn + row * disRow;
        }
    }
}

void Parser::parsePath() {}; // todo

bool Parser::hasFormatSpecifier(const std::string &str) {
    static const std::string formatSpecifiers = "cdfsfeEgGxXo";

    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%') {
            if (i + 1 < str.length() && formatSpecifiers.find(str[i + 1]) != std::string::npos) {
                return true;
            }
        }
    }
    return false;
}

} // namespace MCA2