#include "data_structure.hpp"
#include "preprocess.hpp"
#include "utils.hpp"

#include <array>
#include <cmath>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace MCA2 {

void PreProcessor::calOffsetVectorsFromOneMI(const cv::Mat &image, const cv::Point2i &curCenter,
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

    int rangeEnd = radius + std::round(static_cast<double>(halfSideLength) / sqrt(3));
    int rangeY2[] = {radius, rangeEnd};
    findBestOffset(1, rangeY2[0], rangeY2[1], 1, -1, sqrt(3));  // RTOP
    findBestOffset(2, rangeY2[0], rangeY2[1], 1, 1, sqrt(3));   // RBOT
    findBestOffset(4, rangeY2[0], rangeY2[1], -1, 1, sqrt(3));  // LBOT
    findBestOffset(5, rangeY2[0], rangeY2[1], -1, -1, sqrt(3)); // LTOP
}

void PreProcessor::calOffsetVectors(const cv::Mat &image, SequenceInfo &seqInfo) {
    // count
    std::array<std::unordered_map<std::string, int>, NeighborNum> countMap;

    std::array<cv::Point2i, NeighborNum> tmpOffsets;
    std::array<double, NeighborNum> ssimScores;

    const int threshold = 10;

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
        for (const auto &pair : countMap[i]) {
            std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
            if (pair.second > threshold) {
                size_t commaPos = pair.first.find(',');
                offsetsCandidates[i].push_back(pair);
            }
        }
        std::sort(offsetsCandidates[i].begin(), offsetsCandidates[i].end(),
                  [](const auto &a, const auto &b) { return a.second > b.second; });

        // output
        for (const auto &pair : offsetsCandidates[i]) {
            std::cout << pair.first << ": " << pair.second << std::endl;
        }
    }
}
} // namespace MCA2