#include "postprocess.hpp"
#include "utils.hpp"
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

void PostProcessor::restoreFourCorners(cv::Mat &image, const SequenceInfo &seqInfo) {
    std::cout << "Restoring four corners..." << std::endl;

    for (int i = 1; i < seqInfo.colNum - 1; i++) {
        for (int j = 1; j < seqInfo.rowNum - 2; j++) {
            if (j == seqInfo.rowNum - 2 && seqInfo.colNum % 2 == 1 && j % 2 == 1)
                continue;

            cv::Point2i curCenter(std::round(seqInfo.centers[i * seqInfo.rowNum + j].x),
                                  std::round(seqInfo.centers[i * seqInfo.rowNum + j].y));

            std::array<cv::Point2i, NeighborNum> bestOffsets = findBestOffset(image, curCenter);

            std::array<cv::Mat, CornerNum> cornerMasks =
                getFourCornerMasks(image.size(), curCenter);

            copyTo(image, cornerMasks[0], bestOffsets[3]);
            copyTo(image, cornerMasks[1], bestOffsets[4], bestOffsets[5]);
            copyTo(image, cornerMasks[2], bestOffsets[0]);
            copyTo(image, cornerMasks[3], bestOffsets[1], bestOffsets[2]);
        }
    }
}

std::array<cv::Mat, CornerNum> PostProcessor::getFourCornerMasks(const cv::Size &imageSize,
                                                                 cv::Point2i &center) {

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

    if (srcBoundingBox.x < 0 || srcBoundingBox.y < 0 ||
        srcBoundingBox.x + srcBoundingBox.width > image.cols ||
        srcBoundingBox.y + srcBoundingBox.height > image.rows) {
        std::cerr << "Error: srcROI is out of bounds." << std::endl;
        return;
    }

    cv::Mat srcROI = image(srcBoundingBox); // 从 image 中获取源区域

    srcROI.copyTo(image(dstBoundingBox), dstMask(dstBoundingBox));

    if (offset2 != cv::Point2i(0, 0)) {
        srcBoundingBox = dstBoundingBox + offset2;

        if (srcBoundingBox.x < 0 || srcBoundingBox.y < 0 ||
            srcBoundingBox.x + srcBoundingBox.width > image.cols ||
            srcBoundingBox.y + srcBoundingBox.height > image.rows) {
            std::cerr << "Error: srcROI is out of bounds." << std::endl;
            return;
        }

        cv::Mat srcROI = image(srcBoundingBox);
        int threshold = 30;
        for (int x = 0; x < dstBoundingBox.width; x++) {
            for (int y = 0; y < dstBoundingBox.height; y++) {
                if (dstMask.at<uchar>(dstBoundingBox.y + y, dstBoundingBox.x + x) > 0) {
                    cv::Vec3b pixel1 =
                        image.at<cv::Vec3b>(dstBoundingBox.y + y, dstBoundingBox.x + x);
                    cv::Vec3b pixel2 =
                        image.at<cv::Vec3b>(srcBoundingBox.y + y, srcBoundingBox.x + x);

                    if ((pixel2[0] + pixel2[1] + pixel2[2]) > pixel1[0] + pixel1[1] + pixel1[2]) {
                        image.at<cv::Vec3b>(dstBoundingBox.y + y, dstBoundingBox.x + x) = pixel2;
                    }
                }
            }
        }
    }
}
} // namespace MCA2
