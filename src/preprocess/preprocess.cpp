#include "preprocess.hpp"
#include "data-structure.hpp"
#include "utils.hpp"

#include <array>
#include <cmath>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/fast_math.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/operations.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

namespace MCA2 {
/**

 */
void preprocess(SequenceInfo &seqInfo, TaskInfo &taskInfo) {
    // todo: 一个序列应该计算一次 predict vector 就够了
    int radius = static_cast<int>(seqInfo.diameter) / 2;
    int halfSideLength = std::floor(static_cast<double>(radius) / sqrt(2));
    int sideLength = 2 * halfSideLength + 1;

    for (int i = taskInfo.startFrame; i <= taskInfo.endFrame; i++) {
        // read image
        cv::Mat image = getInputImage(taskInfo.inputPath, i);
        if (image.empty()) {
            std::cout << "[Warning] Image not found: " << taskInfo.inputPath << std::endl;
            continue;
        }

        cv::Mat croppedImage = cv::Mat::zeros(
            cv::Size(seqInfo.colNum * sideLength, seqInfo.rowNum * sideLength), CV_8UC3);

        // temp: analysis
        // analysis(image, seqInfo.centers, static_cast<int>(seqInfo.diameter));
        // analysis2();

        for (int idx = 0; idx < seqInfo.centers.size(); idx++) {
            cv::Point2i center(cvRound(seqInfo.centers[idx].x), cvRound(seqInfo.centers[idx].y));

            cropAndRealign(image, croppedImage, center, idx, seqInfo.colNum, sideLength);

            auto fourCornerMasks = getFourCornerMasks(image.size(), center, radius);
            // cv::Rect patch(center.x - halfSideLength, center.y - halfSideLength, sideLength, sideLength);
            // cv::rectangle(fourCornerMasks[0], patch, cv::Scalar(255), -1);

            cv::Mat test;
            cv::copyTo(image, test, fourCornerMasks[0]);
            
            cv::imwrite(
                "/Users/riverzhao/Project/Codec/0_lvc_codec/Inter-MCA/data/temp/test-getCorner.png",
                test);
        }

        cv::imwrite(taskInfo.outputPath, croppedImage);
    }
};

void cropAndRealign(cv::Mat &rawImage, cv::Mat &croppedImage, cv::Point2i &center, int idx,
                    int colNum, int sideLength) {
    int halfSideLength = sideLength / 2;
    cv::Rect roiRect(center.x - halfSideLength, center.y - halfSideLength, sideLength, sideLength);
    cv::Mat croppedMicroImage = rawImage(roiRect);

    int targetX = (idx % colNum) * sideLength;
    int targetY = (idx / colNum) * sideLength;
    cv::Rect targetROI(targetX, targetY, sideLength, sideLength);
    croppedMicroImage.copyTo(croppedImage(targetROI));
}

void predictFourCorners(cv::Mat &rawImage, PredictInfo &predictInfo, cv::Point2i &center,
                        int radius, int sideLength) {
    int halfSideLength = sideLength / 2;

    cv::Mat mask = cv::Mat::zeros(rawImage.size(), CV_8UC1);
    cv::circle(mask, center, radius, cv::Scalar(255), -1);
    cv::Rect rectPatch(center.x - halfSideLength, center.y - halfSideLength, sideLength,
                       sideLength);
    cv::rectangle(mask, rectPatch, cv::Scalar(0), -1);

    cv::Mat fourCorners;
    cv::copyTo(rawImage, fourCorners, mask);

    cv::imwrite("/Users/riverzhao/Project/Codec/0_lvc_codec/Inter-MCA/data/temp/corners-one-MI.png",
                fourCorners);
}

std::array<cv::Mat, 4> getFourCornerMasks(const cv::Size &imageSize, cv::Point2i &center, int radius) {
    int halfSideLength = std::floor(static_cast<double>(radius) / sqrt(2));
    int sideLength = 2 * halfSideLength + 1, diameter = radius * 2 + 1;

    std::array<cv::Mat, 4> fourCornersMasks;
    for (auto &mask : fourCornersMasks) {
        mask = cv::Mat::zeros(imageSize, CV_8UC1);
        cv::circle(mask, center, radius, cv::Scalar(255), -1);
    }

    std::array<cv::Rect, 4> rectMasks;
    int longEdge = diameter, shortEdge = halfSideLength + radius + 1;
    rectMasks[0] = cv::Rect(center.x - radius, center.y - halfSideLength, longEdge, shortEdge);
    rectMasks[1] = cv::Rect(center.x - radius, center.y - radius, shortEdge, longEdge);
    rectMasks[2] = cv::Rect(center.x - radius, center.y - radius, longEdge, shortEdge);
    rectMasks[3] = cv::Rect(center.x - halfSideLength, center.y - radius, shortEdge, longEdge);
    for (int i = 0; i < 4; i++) {
        cv::rectangle(fourCornersMasks[i], rectMasks[i], cv::Scalar(0), -1);
    }

    return fourCornersMasks;
}

void analysis(const cv::Mat &image, const std::vector<cv::Point2d> &centers, int diameter) {
    cv::Mat processedImage;

    int radius = diameter / 2;
    int halfSideLength = std::floor(static_cast<double>(radius) / sqrt(2));

    cv::Mat maskCircles = cv::Mat::zeros(image.size(), CV_8UC1);
    for (auto &center_d : centers) {
        cv::Point2i center(std::round(center_d.x), std::round(center_d.y));
        cv::circle(maskCircles, center, radius, cv::Scalar(255), -1);
    }

    cv::copyTo(image, processedImage, maskCircles);
    cv::imwrite("/Users/riverzhao/Project/Codec/0_lvc_codec/Inter-MCA/data/temp/MIs.png",
                processedImage);

    // cv::Mat maskRects = cv::Mat::ones(image.size(), CV_8UC1);
    // int biasX = 1, biasY = 1;

    // for (auto &center_d : centers) {
    //     cv::Point2i center(std::round(center_d.x) + biasX, std::round(center_d.y) + biasY);
    //     cv::Rect rect(center.x - halfSideLength, center.y - halfSideLength, sideLength,
    //     sideLength); cv::rectangle(maskRects, rect, cv::Scalar(0), -1);
    // }
    for (auto &center_d : centers) {
        cv::Point2i center(std::round(center_d.x), std::round(center_d.y));
        cv::rectangle(processedImage,
                      cv::Point2i(center.x - halfSideLength, center.y - halfSideLength),
                      cv::Point2i(center.x + halfSideLength, center.y + halfSideLength),
                      cv::Scalar(255, 0, 0), -1);
    }

    cv::imwrite("/Users/riverzhao/Project/Codec/0_lvc_codec/Inter-MCA/data/temp/four-corners.png",
                processedImage);
}

void analysis2() {
    int w = 100, h = 100, diameter = 39;
    int radius = diameter / 2;

    int halfSideLength = std::round(static_cast<double>(radius) / sqrt(2));

    std::cout << "diameter: " << diameter << std::endl;
    std::cout << "radius: " << radius << std::endl;

    cv::Point2d center(50, 50);
    cv::Mat canvas = cv::Mat::zeros(cv::Size2d(w, h), CV_8UC1);
    cv::circle(canvas, center, radius, cv::Scalar(255), -1);

    cv::Point2d ltop(center.x - halfSideLength, center.y - halfSideLength);
    cv::Point2d rbot(center.x + halfSideLength, center.y + halfSideLength);
    cv::rectangle(canvas, ltop, rbot, cv::Scalar(150), -1);

    int count = 0;
    for (int i = 1; i < w + 1; i++) {
        if (static_cast<int>(canvas.at<uchar>(50, i)) != 0) {
            // canvas.at<uchar>(50, i) = 100;
            count++;
        }
    }
    std::cout << "count: " << count << std::endl;

    cv::imwrite("/Users/riverzhao/Project/Codec/0_lvc_codec/Inter-MCA/data/temp/test.png", canvas);
}

} // namespace MCA2