/*
Read calibration files and calculate center points coordinates
 */
#pragma once

#include <opencv2/core/types.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace MCA2 {
struct TaskInfo {
    std::string calibrationFilePath;
    std::string inputPath;
    std::string outputPath;
    int startFrame, endFrame;
    int height, width;

    TaskInfo() {};
    TaskInfo(std::string &calibFile, std::string &input, std::string &output, int start, int end,
             int h, int w)
        : calibrationFilePath(calibFile), inputPath(input), outputPath(output), startFrame(start),
          endFrame(end), height(h), width(w) {}
};

struct SequenceInfo {
    int width, height;
    float diameter;
    float rotationAngle;
    int rows, cols;
    cv::Point2f cornerPoints[4];
    std::vector<cv::Point2f> centers;

    SequenceInfo() {};
    SequenceInfo(int w, int h, float d, float rAngle, int r, int c, const cv::Point2f *corners,
                 const std::vector<cv::Point2f> &cts)
        : width(w), height(h), diameter(d), rotationAngle(rAngle), rows(r), cols(c), centers(cts) {
        std::copy(corners, corners + 4, cornerPoints);
    }
};

class Parser {
private:
    int parseConfigFile(std::string &cfgFilePath,
                        std::unordered_map<std::string, std::string>& config);
    int parseCalibXMLFile(std::string &calibXMLFilePath);

public:
    TaskInfo taskInfo;
    SequenceInfo sequenceInfo;

    Parser(std::string &cfgFilePath);
};

} // namespace MCA2