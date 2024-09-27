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

    TaskInfo() {};
    TaskInfo(std::string &calibFile, std::string &input, std::string &output, int start, int end)
        : calibrationFilePath(calibFile), inputPath(input), outputPath(output), startFrame(start),
          endFrame(end) {}
};

struct SequenceInfo {
    int width, height, rowNum, colNum;
    int direction; // 0 is vertical, 1 is horizontal
    double diameter;
    cv::Point2d ltop, rtop, lbot, rbot;

    std::vector<cv::Point2d> centers;

    // Set default value
    SequenceInfo()
        : ltop(cv::Point2d(0, 0)), rtop(cv::Point2d(0, 0)), lbot(cv::Point2d(0, 0)),
          rbot(cv::Point2d(0, 0)) {};
};

using PredictInfo = std::vector<std::array<cv::Point2i, 4>>;
} // namespace MCA2
