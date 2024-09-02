#include "preprocess.hpp"
#include <iostream>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <string>

namespace MCA2 {
void preprocess(SequenceInfo &seqInfo, TaskInfo &taskInfo) {
    for (int i = taskInfo.startFrame; i <= taskInfo.endFrame; i++) {
        cv::Mat image = getInputImage(taskInfo.inputPath, i);
        if (image.empty())
            continue;

        for (const auto &center2d : seqInfo.centers) {
            
        }
    }
};

cv::Mat getInputImage(std::string &pathPattern, int idx) {
    char filePath[256];
    std::snprintf(filePath, sizeof(filePath), pathPattern.c_str(), idx);
    cv::Mat image = cv::imread(filePath, cv::IMREAD_COLOR);

    if (image.empty()) {
        std::cerr << "Error: Unable to load image from " << filePath << std::endl;
    }

    return image;
}

} // namespace MCA2