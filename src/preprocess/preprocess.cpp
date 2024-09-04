#include "preprocess.hpp"
#include "data-structure.hpp"
#include "utils.hpp"

#include <cmath>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/fast_math.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

namespace MCA2 {
void preprocess(const SequenceInfo &seqInfo, const TaskInfo &taskInfo) {
    for (int i = taskInfo.startFrame; i <= taskInfo.endFrame; i++) {
        // read image
        cv::Mat image = getInputImage(taskInfo.inputPath, i);
        if (image.empty())
            continue;

        // 
        int sideLength = std::round(seqInfo.diameter / std::sqrt(2));

        std::cout << "side length: " << sideLength << " " << std::endl;

        cv::Mat croppedImage = cv::Mat::zeros(
            cv::Size(seqInfo.colNum * sideLength, seqInfo.rowNum * sideLength), CV_8UC3);
        PredictInfo predict(seqInfo.centers.size());

        // temp: analysis
        // analysis(image, seqInfo.centers, seqInfo.diameter);

        for (int idx = 0; idx < seqInfo.centers.size(); idx++) {
            cropAndRealign(image, croppedImage, seqInfo, idx, sideLength);
        }

        cv::imwrite(taskInfo.outputPath, croppedImage);
    }
};

void cropAndRealign(cv::Mat &image, cv::Mat &croppedImage, SequenceInfo &seqInfo, int idx,
                    int sideLength) {

    int halfSideLength = std::ceil(static_cast<double>(sideLength) / 2);
    cv::Point2i center(std::round(seqInfo.centers[idx].x), std::round(seqInfo.centers[idx].y));

    cv::Rect roiRect(center.x - halfSideLength, center.y - halfSideLength, sideLength, sideLength);
    cv::Mat croppedMicroImage = image(roiRect);

    int targetX = (idx % seqInfo.colNum) * sideLength;
    int targetY = (idx / seqInfo.colNum) * sideLength;

    cv::Rect targetROI(targetX, targetY, sideLength, sideLength);
    croppedMicroImage.copyTo(croppedImage(targetROI));
}

void analysis(const cv::Mat &image, const std::vector<cv::Point2d> &centers, double diameter) {
    cv::Mat MIs, corners;

    int sideLength = std::round(diameter / std::sqrt(2));
    int halfSideLength = std::ceil(static_cast<double>(sideLength) / 2);
    int radius = std::round(diameter / 2);

    cv::Mat maskCircles = cv::Mat::zeros(image.size(), CV_8UC1);
    for (auto &center_d : centers) {
        cv::Point2i center(std::round(center_d.x), std::round(center_d.y));
        cv::circle(maskCircles, center, radius, cv::Scalar(255), -1);
    }

    cv::copyTo(image, MIs, maskCircles);
    cv::imwrite("/Users/riverzhao/Project/Codec/0_lvc_codec/Inter-MCA/data/temp/MIs.png", MIs);

    cv::Mat maskRects = cv::Mat::ones(image.size(), CV_8UC1);
    for (auto &center_d : centers) {
        cv::Point2i center(std::round(center_d.x), std::round(center_d.y));
        cv::Rect rect(center.x - halfSideLength, center.y - halfSideLength, sideLength, sideLength);
        cv::rectangle(maskRects, rect, cv::Scalar(0), -1);
    }

    cv::copyTo(MIs, corners, maskRects);
    cv::imwrite("/Users/riverzhao/Project/Codec/0_lvc_codec/Inter-MCA/data/temp/corners.png",
                corners);
}

} // namespace MCA2