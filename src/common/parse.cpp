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

    taskInfo = TaskInfo(config["Calibration_xml"], config["RawImage_Path"], config["Output_Path"],
                        std::stoi(config["start_frame"]), std::stoi(config["end_frame"]),
                        std::stoi(config["height"]), std::stoi(config["width"]));
    configFile.close();

    return 0;
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
    seqInfo.rotationAngle = root.child("rotation").text().as_double();

    const pugi::xml_node centerNode = root.child("centers");
    seqInfo.rowNum = centerNode.child("rows").text().as_int();
    seqInfo.colNum = centerNode.child("cols").text().as_int();

    seqInfo.ltop = cv::Point2d(centerNode.child("ltop").child("x").text().as_double(),
                               centerNode.child("ltop").child("y").text().as_double());
    seqInfo.rtop = cv::Point2d(centerNode.child("rtop").child("x").text().as_double(),
                               centerNode.child("rtop").child("y").text().as_double());
    seqInfo.lbot = cv::Point2d(centerNode.child("lbot").child("x").text().as_double(),
                               centerNode.child("lbot").child("y").text().as_double());

    if (!centerNode.child("rbot").empty()) {
        seqInfo.isThreePoints = false;
        seqInfo.rbot = cv::Point2d(centerNode.child("rbot").child("x").text().as_double(),
                                   centerNode.child("rbot").child("y").text().as_double());
    }

    calAllCenterPoints(seqInfo);

    return 0;
}

void Parser::calAllCenterPoints(SequenceInfo &seqInfo) {
    double xBias = seqInfo.diameter / 2 * sqrt(3);
    double yBias = seqInfo.diameter / 2;
    cv::Point2d colGap, rowGap;
    cv::Point2d ltopEven(seqInfo.ltop.x + xBias, seqInfo.ltop.y + yBias);

    if (seqInfo.isThreePoints) {
        colGap = (seqInfo.rtop - ltopEven) / (seqInfo.colNum / 2 - 1);
        rowGap = (seqInfo.lbot - seqInfo.ltop) / (seqInfo.rowNum - 1);

    } else {
        // todo
    }

    seqInfo.centers.resize(seqInfo.colNum * seqInfo.rowNum);
    seqInfo.centers[0] = seqInfo.ltop;
    seqInfo.centers[1] = ltopEven;

    std::cout << "colGap: " << colGap.x << ", " << colGap.y << "\n"
              << "rowGap: " << rowGap.x << ", " << rowGap.y << std::endl;

    std::cout << "Number of center points: " << seqInfo.centers.size() << std::endl;
    std::cout << "ltop: " << seqInfo.centers[0].x << ", " << seqInfo.centers[0].y << "\n"
              << "ltopOdd: " << seqInfo.centers[1].x << ", " << seqInfo.centers[1].y << std::endl;

    for (int i = 2; i < seqInfo.colNum; i++) {
        seqInfo.centers[i] = seqInfo.centers[i - 2] + colGap;
    }
    for (int i = 1; i < seqInfo.rowNum; i++) {
        for (int j = 0; j < seqInfo.colNum; j++) {
            seqInfo.centers[i * seqInfo.colNum + j] =
                seqInfo.centers[(i - 1) * seqInfo.colNum + j] + rowGap;
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