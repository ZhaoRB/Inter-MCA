#include "preprocess.hpp"
#include "data-structure.hpp"
#include <cmath>
#include <iostream>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <string>

namespace MCA2 {
void preprocess(SequenceInfo &seqInfo, TaskInfo &taskInfo) {
    for (int i = taskInfo.startFrame; i <= taskInfo.endFrame; i++) {
        cv::Mat image = getInputImage(taskInfo.inputPath, i);
        if (image.empty())
            continue;

        double radius_d = seqInfo.diameter / 2, halfEdge_d = radius_d / std::sqrt(2);
        int radius = std::floor(radius_d), halfEdge = std::ceil(halfEdge_d);
        int edge = halfEdge * 2 + 1;

        cv::Mat croppedImage =
            cv::Mat::zeros(cv::Size(seqInfo.colNum * edge, seqInfo.rowNum * edge), CV_8UC3);
        PredictInfo predict(seqInfo.centers.size());

        for (int idx = 0; idx < seqInfo.centers.size(); idx++) {
            cv::Point2i center(std::round(seqInfo.centers[idx].x),
                               std::round(seqInfo.centers[idx].y));
            cv::Point2i leftTop(center.x - halfEdge, center.y - halfEdge);
            cv::Point2i rightBottom(center.x + halfEdge, center.y + halfEdge);

            cv::Rect roiRect(leftTop, rightBottom);
            cv::Mat croppedMicroImage = image(roiRect);

            std::cout << "Cropped micro image size: " << roiRect.size() << std::endl;
            cv::imshow("name", croppedMicroImage);
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

// cv::Mat extractMicroImage(cv::Mat &image, const cv::Point2d &center, double diameter) {
//     cv::Mat mask = cv::Mat::zeros(image.size(), CV_8UC1);

//     cv::circle(mask, center, static_cast<int>(diameter / 2), cv::Scalar(255), -1);

//     cv::Mat roi;
//     image.copyTo(roi, mask);

//     std::cout << "roi size: " << roi.size() << std::endl;

//     cv::imwrite("/Users/riverzhao/Project/Codec/0_lvc_codec/Inter-MCA/data/temp/roi.png", roi);

//     return roi;
// }
} // namespace MCA2