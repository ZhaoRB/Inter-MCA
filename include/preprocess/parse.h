/*
Read calibration files and calculate center points coordinates
 */
#pragma once

#include <opencv2/core/types.hpp>
#include <string>
#include <vector>

namespace MCA2 {
struct TaskInfo {
    std::string calibrationFilePath;
    std::string inputPath;
    std::string outputPath;
    int startFrame, endFrame;

    TaskInfo();

    TaskInfo(std::string &calibFile, std::string &input, std::string &output, int start, int end)
        : calibrationFilePath(calibFile), inputPath(input), outputPath(output), startFrame(start),
          endFrame(end) {}
};

struct SequenceInfo {
    int width, height;
    float diameter;
    float rotationAngle;
    int rows, cols;
    cv::Point2f cornerPoints[4];
    std::vector<cv::Point2f> centers;

    SequenceInfo();

    SequenceInfo(int w, int h, float d, float rAngle, int r, int c, const cv::Point2f *corners,
                 const std::vector<cv::Point2f> &cts)
        : width(w), height(h), diameter(d), rotationAngle(rAngle), rows(r), cols(c), centers(cts) {
        std::copy(corners, corners + 4, cornerPoints);
    }
};

class Parser {
public:
    TaskInfo taskInfo;
    SequenceInfo sequenceInfo;

    int parseConfigFile(std::string &cfgFilePath);
    int parseCalibXMLFile(std::string &calibXMLFilePath);
};

} // namespace MCA2