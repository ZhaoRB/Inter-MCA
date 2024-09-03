/*
Define data structures
 */
#pragma once

#include <array>
#include <opencv2/core/types.hpp>
#include <string>
#include <vector>

namespace MCA2 {

struct TaskInfo {
    std::string calibrationFilePath;
    std::string inputPath;
    std::string outputPath;
    int startFrame, endFrame;
    int height, width;

    // std::vector<std::string> inputNames;

    TaskInfo() {};
    TaskInfo(std::string &calibFile, std::string &input, std::string &output, int start, int end,
             int h, int w)
        : calibrationFilePath(calibFile), inputPath(input), outputPath(output), startFrame(start),
          endFrame(end), height(h), width(w) {}
};

struct SequenceInfo {
    int width, height, rowNum, colNum;
    double diameter, rotationAngle;
    cv::Point2d ltop, rtop, lbot, rbot;

    bool isThreePoints;
    std::vector<cv::Point2d> centers;

    SequenceInfo()
        : ltop(cv::Point2d(0, 0)), rtop(cv::Point2d(0, 0)), lbot(cv::Point2d(0, 0)),
          rbot(cv::Point2d(0, 0)), isThreePoints(true) {};
};

using PredictInfo = std::vector<std::array<cv::Point2i, 4>>;
} // namespace MCA2
