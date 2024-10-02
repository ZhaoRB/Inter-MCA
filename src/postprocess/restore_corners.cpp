#include "postprocess.hpp"
#include "utils.hpp"
#include <array>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace MCA2 {
void PostProcessor::parseSupInfo(const std::string &supInfoPath) {
    std::ifstream supInfoFile(supInfoPath);
    std::string line;

    int idx = 0;
    while (std::getline(supInfoFile, line)) {
        std::stringstream ss(line);
        std::string point;

        while (std::getline(ss, point, ';')) {
            if (!point.empty()) {
                std::stringstream pointStream(point);
                std::string xStr, yStr;
                std::getline(pointStream, xStr, ',');
                std::getline(pointStream, yStr, ',');
                offsetCandidates[idx].emplace_back(std::stoi(xStr), std::stoi(yStr));
            }
        }

        idx++;
    }
}

std::array<cv::Point2i, NeighborNum> PostProcessor::findBestOffset(const cv::Mat &image,
                                                                   const cv::Point2i &center) {
    std::array<cv::Point2i, NeighborNum> bestOffsets;
    std::array<double, NeighborNum> ssimScores;

    std::array<cv::Rect, NeighborNum> srcROIs = {
        cv::Rect(center.x - halfSideLength, center.y + 1, sideLength,
                 halfSideLength), // TOP
        cv::Rect(center.x - halfSideLength, center.y + 1, halfSideLength,
                 halfSideLength), // RTOP
        cv::Rect(center.x - halfSideLength, center.y - halfSideLength, halfSideLength,
                 halfSideLength), // RBOT
        cv::Rect(center.x - halfSideLength, center.y - halfSideLength, sideLength,
                 halfSideLength), // BOT
        cv::Rect(center.x + 1, center.y - halfSideLength, halfSideLength,
                 halfSideLength),                                              // LBOT
        cv::Rect(center.x + 1, center.y + 1, halfSideLength, halfSideLength)}; // LTOP

    for (int i = 0; i < NeighborNum; i++) {
        ssimScores[i] = 0;
        for (auto offset : offsetCandidates[i]) {
            cv::Rect tgtROI = srcROIs[i] + offset;
            double ssim = calculateSSIM(image(srcROIs[i]), image(tgtROI));
            if (ssim > ssimScores[i]) {
                ssimScores[i] = ssim;
                bestOffsets[i] = offset;
            }
        }
    }

    return bestOffsets;
}

void PostProcessor::restoreFourCorners(cv::Mat &dstImage, const SequenceInfo &seqInfo) {
    std::cout << "Restoring four corners..." << std::endl;

    // restore main part
    for (int i = 1; i < seqInfo.colNum - 1; i++) {
        for (int j = 1; j < seqInfo.rowNum - 1; j++) {

            cv::Point2i curCenter(std::round(seqInfo.centers[i * seqInfo.rowNum + j].x),
                                  std::round(seqInfo.centers[i * seqInfo.rowNum + j].y));

            std::array<cv::Point2i, NeighborNum> bestOffsets = findBestOffset(dstImage, curCenter);

            std::array<cv::Mat, CornerNum> cornerMasks =
                getFourCornerMasks(dstImage.size(), curCenter);

            copyTo(dstImage, cornerMasks[0], bestOffsets[3]);                 // top
            copyTo(dstImage, cornerMasks[1], bestOffsets[4], bestOffsets[5]); // right
            copyTo(dstImage, cornerMasks[2], bestOffsets[0]);                 // bottom
            copyTo(dstImage, cornerMasks[3], bestOffsets[1], bestOffsets[2]); // left

            // lumaCompensation(dstImage, curCenter);
        }
    }

    // restore edge MI and half MI
    dstImage = expandImage(dstImage, radius, 0, radius, 0);
    // first column and last column
    for (int row = 0; row < seqInfo.rowNum; ++row) {
        restoreCornersOfEdgeMI(dstImage, cv::Point2i(std::round(seqInfo.centers[row].x),
                                                     std::round(seqInfo.centers[row].y + radius)));
        restoreCornersOfEdgeMI(
            dstImage,
            cv::Point2i(std::round(seqInfo.centers[(seqInfo.colNum - 1) * seqInfo.rowNum + row].x),
                        std::round(seqInfo.centers[(seqInfo.colNum - 1) * seqInfo.rowNum + row].y +
                                   radius)));
    }
    // first row and last row and half MI
    for (int col = 0; col < seqInfo.colNum; ++col) {
        restoreCornersOfEdgeMI(
            dstImage, cv::Point2i(std::round(seqInfo.centers[col * seqInfo.rowNum].x),
                                  std::round(seqInfo.centers[col * seqInfo.rowNum].y + radius)));
        restoreCornersOfEdgeMI(
            dstImage,
            cv::Point2i(std::round(seqInfo.centers[(col + 1) * seqInfo.rowNum - 1].x),
                        std::round(seqInfo.centers[(col + 1) * seqInfo.rowNum - 1].y + radius)));

        if (seqInfo.colNum % 2 == 1) {
            restoreCornersOfEdgeMI(
                dstImage,
                cv::Point2i(
                    std::round(seqInfo.centers[(col + 1) * seqInfo.rowNum - 2].x),
                    std::round(radius + seqInfo.centers[(col + 1) * seqInfo.rowNum - 2].y)));
        }

        if (col % 2 == 0) {
            restoreCornersOfEdgeMI(
                dstImage, cv::Point2i(std::round(seqInfo.centers[(col + 1) * seqInfo.rowNum - 1].x),
                                      std::round(seqInfo.centers[(col + 1) * seqInfo.rowNum - 1].y +
                                                 radius + seqInfo.diameter)));
        } else {
            restoreCornersOfEdgeMI(
                dstImage, cv::Point2i(std::round(seqInfo.centers[col * seqInfo.rowNum].x),
                                      std::round(seqInfo.centers[col * seqInfo.rowNum].y + radius -
                                                 seqInfo.diameter - 1))); // diameter - 1
        }
    }
    dstImage = cropImage(dstImage, radius, 0, radius, 0);
}

void PostProcessor::restoreCornersOfEdgeMI(cv::Mat &image, const cv::Point2i &center) {
    cv::Mat mask = cv::Mat::zeros(image.size(), CV_8UC1);
    cv::circle(mask, center, radius, cv::Scalar(255), -1);

    cv::Point2i patchLtop(center.x - halfSideLength, center.y - halfSideLength);
    cv::Point2i patchRbot(center.x + halfSideLength, center.y + halfSideLength);

    auto fillEdge = [&](const cv::Point2i &start, const cv::Point2i &step) {
        cv::Vec3b pixel = image.at<cv::Vec3b>(start);
        cv::Point2i tgt = start + step;
        while (mask.at<uchar>(tgt) != 0) {
            image.at<cv::Vec3b>(tgt) = pixel;
            tgt += step;
        }
    };

    // top & bottom
    for (int x = patchLtop.x; x <= patchRbot.x; ++x) {
        fillEdge(cv::Point2i(x, patchLtop.y), cv::Point2i(0, -1)); // top
        fillEdge(cv::Point2i(x, patchRbot.y), cv::Point2i(0, 1));  // bottom
    }

    // left & right
    for (int y = patchLtop.y; y <= patchRbot.y; ++y) {
        fillEdge(cv::Point2i(patchLtop.x, y), cv::Point2i(-1, 0)); // left
        fillEdge(cv::Point2i(patchRbot.x, y), cv::Point2i(1, 0));  // right
    }
}

std::array<cv::Mat, CornerNum> PostProcessor::getFourCornerMasks(const cv::Size &imageSize,
                                                                 const cv::Point2i &center) {

    std::array<cv::Mat, CornerNum> cornerMasks;
    for (auto &mask : cornerMasks) {
        mask = cv::Mat::zeros(imageSize, CV_8UC1);
        cv::circle(mask, center, radius, cv::Scalar(255), -1);
    }

    std::array<cv::Rect, CornerNum> rectMasks;
    int longEdge = radius * 2 + 1, shortEdge = halfSideLength + radius + 1;
    rectMasks = {
        cv::Rect(center.x - radius, center.y - halfSideLength, longEdge, shortEdge),  // top
        cv::Rect(center.x - radius, center.y - radius, shortEdge, longEdge),          // right
        cv::Rect(center.x - radius, center.y - radius, longEdge, shortEdge),          // bot
        cv::Rect(center.x - halfSideLength, center.y - radius, shortEdge, longEdge)}; // left

    for (int i = 0; i < CornerNum; i++) {
        cv::rectangle(cornerMasks[i], rectMasks[i], cv::Scalar(0), -1);
    }

    return cornerMasks;
}

void PostProcessor::copyTo(cv::Mat &image, cv::Mat &dstMask, const cv::Point2i &offset1,
                           const cv::Point2i &offset2) {
    cv::Rect dstBoundingBox = cv::boundingRect(dstMask);
    cv::Rect srcBoundingBox = dstBoundingBox + offset1;

    cv::Mat srcROI = image(srcBoundingBox);
    srcROI.copyTo(image(dstBoundingBox), dstMask(dstBoundingBox));

    if (offset2 != cv::Point2i(0, 0)) {
        srcBoundingBox = dstBoundingBox + offset2;
        srcROI = image(srcBoundingBox);

        int threshold = 30;
        for (int x = 0; x < dstBoundingBox.width; x++) {
            for (int y = 0; y < dstBoundingBox.height; y++) {
                if (dstMask.at<uchar>(dstBoundingBox.y + y, dstBoundingBox.x + x) == 0)
                    continue;

                cv::Vec3b &pixel1 = image.at<cv::Vec3b>(dstBoundingBox.y + y, dstBoundingBox.x + x);
                cv::Vec3b pixel2 = image.at<cv::Vec3b>(srcBoundingBox.y + y, srcBoundingBox.x + x);

                if ((pixel2[0] + pixel2[1] + pixel2[2]) > pixel1[0] + pixel1[1] + pixel1[2])
                    pixel1 = pixel2;
            }
        }
    }
}

void PostProcessor::lumaCompensation(cv::Mat &image, const cv::Point2i &center) {
    // calculate luminance decay coefficients
    double lumaCenter = calculateLuma(image.at<cv::Vec3b>(center));
    const int DIRECTIONS = 4;

    std::unordered_map<int, double> coefficentMap[DIRECTIONS];
    int start = std::round(static_cast<double>(halfSideLength) / sqrt(2));
    int end = halfSideLength;

    std::vector<cv::Point2i> offsets = {{-1, -1}, {1, -1}, {1, 1}, {-1, 1}};

    for (int i = start; i < end; ++i) {
        int distance = std::round(i * sqrt(2));

        for (int j = 0; j < DIRECTIONS; ++j) {
            double lumaCur = calculateLuma(image.at<cv::Vec3b>(center + offsets[j] * i));
            coefficentMap[j][distance] = lumaCur / lumaCenter;
        }
    }

    // luma compensation
    cv::Mat mask = cv::Mat::zeros(image.size(), CV_8UC1);
    cv::circle(mask, center, radius, cv::Scalar(255), -1);
    cv::rectangle(
        mask,
        cv::Rect(center.x - halfSideLength, center.y - halfSideLength, sideLength, sideLength),
        cv::Scalar(0), -1);

    std::vector<cv::Point2i> ltops = {{center.x - radius, center.y - radius},
                                      {center.x, center.y - radius},
                                      {center.x, center.y},
                                      {center.x - radius, center.y}};

    for (int i = 0; i < DIRECTIONS; i++) {
        // std::cout << i << std::endl;
        // for (const auto &pair : coefficentMap[i]) {
        //     int key = pair.first;
        //     double value = pair.second;
        //     std::cout << "key: " << key << " value: " << value << std::endl;
        // }

        for (int x = 0; x <= radius; x++) {
            for (int y = 0; y <= radius; y++) {
                cv::Point2i cur(ltops[i].x + x, ltops[i].y + y);
                if (mask.at<uchar>(cur) == 0)
                    continue;

                cv::Vec3b &pixel = image.at<cv::Vec3b>(cur);
                int dis = std::round(calculateDistance(cur, center));
                while (coefficentMap[i].count(dis) == 0) {
                    dis--;
                }
                pixel = pixel * coefficentMap[i][dis];
            }
        }
    }
}
} // namespace MCA2
