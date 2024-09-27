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
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace MCA2 {

void Preprocessor::preprocess(SequenceInfo &seqInfo, TaskInfo &taskInfo) {
    // 1. 计算 offset vector（edge micro image 不计算）
    cv::Mat firstFrame = cv::imread(getPath(taskInfo.inputPath, 0));
    calOffsetVectors(firstFrame, seqInfo);

    // for (int i = taskInfo.startFrame; i <= taskInfo.endFrame; i++) {
    //     // read image
    //     cv::Mat image = cv::imread(getPath(taskInfo.inputPath, i));
    //     if (image.empty()) {
    //         std::cout << "[Warning] Image not found: " << taskInfo.inputPath << std::endl;
    //         continue;
    //     }

    //     // crop and realign
    //     cv::Mat croppedImage = cv::Mat::zeros(
    //         cv::Size(seqInfo.colNum * sideLength, seqInfo.rowNum * sideLength), CV_8UC3);

    //     for (int idx = 0; idx < seqInfo.centers.size(); idx++) {
    //         cv::Point2i center(cvRound(seqInfo.centers[idx].x), cvRound(seqInfo.centers[idx].y));
    //         cropAndRealign(image, croppedImage, center, idx, seqInfo.colNum, sideLength);
    //     }

    //     // todo: 这个 filename 应该还得改一下
    //     cv::imwrite(getPath(taskInfo.outputPath, i), croppedImage);
    // }
};

void Preprocessor::calOffsetVectorsFromOneMI(const cv::Mat &image, const cv::Point2i &curCenter,
                                             std::array<double, NeighborNum> &ssimScores,
                                             std::array<cv::Point2i, NeighborNum> &tmpOffsets) {
    std::array<cv::Rect, NeighborNum> roiRects = {
        cv::Rect(curCenter.x - halfSideLength, curCenter.y + 1, sideLength,
                 halfSideLength), // TOP
        cv::Rect(curCenter.x - halfSideLength, curCenter.y + 1, halfSideLength,
                 halfSideLength), // RTOP
        cv::Rect(curCenter.x - halfSideLength, curCenter.y - halfSideLength, halfSideLength,
                 halfSideLength), // RBOT
        cv::Rect(curCenter.x - halfSideLength, curCenter.y - halfSideLength, sideLength,
                 halfSideLength), // BOT
        cv::Rect(curCenter.x + 1, curCenter.y - halfSideLength, halfSideLength,
                 halfSideLength),                                                    // LBOT
        cv::Rect(curCenter.x + 1, curCenter.y + 1, halfSideLength, halfSideLength)}; // LTOP

    auto findBestOffset = [&](int idx, int startY, int endY, int signX, int signY, double factor) {
        for (int biasY = startY; biasY <= endY; biasY++) {
            for (int bias = -1; bias < 2; bias++) {
                int biasX = static_cast<int>(factor * biasY) + bias;
                cv::Point2i offset(signX * biasX, signY * biasY);
                cv::Rect targetROI = roiRects[idx] + offset;
                double ssim = calculateSSIM(image(roiRects[idx]), image(targetROI));
                if (ssim > ssimScores[idx]) {
                    ssimScores[idx] = ssim;
                    tmpOffsets[idx] = offset;
                }
            }
        }
    };

    int rangeY1[] = {2 * radius, 2 * radius + halfSideLength};
    findBestOffset(0, rangeY1[0], rangeY1[1], 0, -1, 1.0); // TOP
    findBestOffset(3, rangeY1[0], rangeY1[1], 0, 1, 1.0);  // BOT

    int rangeY2[] = {radius, static_cast<int>(1.5 * radius)};
    findBestOffset(1, rangeY2[0], rangeY2[1], 1, -1, sqrt(3));  // RTOP
    findBestOffset(2, rangeY2[0], rangeY2[1], 1, 1, sqrt(3));   // RBOT
    findBestOffset(4, rangeY2[0], rangeY2[1], -1, 1, sqrt(3));  // LBOT
    findBestOffset(5, rangeY2[0], rangeY2[1], -1, -1, sqrt(3)); // LTOP
}

void Preprocessor::calOffsetVectors(const cv::Mat &image, SequenceInfo &seqInfo) {
    // count
    std::array<std::unordered_map<std::string, int>, NeighborNum> countMap;

    std::array<cv::Point2i, NeighborNum> tmpOffsets;
    std::array<double, NeighborNum> ssimScores;

    for (int i = 1; i < seqInfo.colNum - 1; i++) {
        for (int j = 1; j < seqInfo.rowNum - 1; j++) {
            std::fill(ssimScores.begin(), ssimScores.end(), 0);

            cv::Point2i curCenter(std::round(seqInfo.centers[i * seqInfo.rowNum + j].x),
                                  std::round(seqInfo.centers[i * seqInfo.rowNum + j].y));

            calOffsetVectorsFromOneMI(image, curCenter, ssimScores, tmpOffsets);

            for (int i = 0; i < NeighborNum; i++) {
                std::ostringstream oss;
                oss << tmpOffsets[i].x << "," << tmpOffsets[i].y;
                std::string key = oss.str();
                countMap[i][key] += 1;
            }
        }
    }

    std::vector<std::string> pos = {"top", "rtop", "rbot", "bot", "lbot", "ltop"};
    for (int i = 0; i < NeighborNum; i++) {
        std::cout << pos[i] << ":" << std::endl;
        int max = 0;
        for (const auto &pair : countMap[i]) {
            std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
            if (pair.second > max) {
                max = pair.second;
                size_t commaPos = pair.first.find(',');
                offsets[i].x = std::stoi(pair.first.substr(0, commaPos));
                offsets[i].y = std::stoi(pair.first.substr(commaPos + 1));
            }
        }
        std::cout << offsets[i].x << ", " << offsets[i].y << std::endl;
    }
}

// MCA
void Preprocessor::cropAndRealign(cv::Mat &rawImage, cv::Mat &croppedImage, cv::Point2i &center,
                                  int idx, int colNum, int sideLength) {
    int halfSideLength = sideLength / 2;
    cv::Rect roiRect(center.x - halfSideLength, center.y - halfSideLength, sideLength, sideLength);
    cv::Mat croppedMicroImage = rawImage(roiRect);

    int targetX = (idx % colNum) * sideLength;
    int targetY = (idx / colNum) * sideLength;
    cv::Rect targetROI(targetX, targetY, sideLength, sideLength);
    croppedMicroImage.copyTo(croppedImage(targetROI));
}

} // namespace MCA2