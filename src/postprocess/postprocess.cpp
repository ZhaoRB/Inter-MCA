#include "postprocess.hpp"
#include "utils.hpp"
#include <cstdlib>
#include <iostream>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace MCA2 {
void postprocess(SequenceInfo &seqInfo, TaskInfo &taskInfo) {
    for (int i = taskInfo.startFrame; i <= taskInfo.endFrame; i++) {
        // read image
        cv::Mat preprocessedImage = getInputImage(taskInfo.inputPath, i);
        if (preprocessedImage.empty()) {
            std::cout << "[Warning] Image not found: " << taskInfo.inputPath << std::endl;
            continue;
        }

        // todo: width 和 height 存在哪里？哪个对象中？
        cv::Mat restoredImage = cv::Mat::zeros(cv::Size(taskInfo.width, taskInfo.height), CV_8UC3);
        // std::cout << "Width (cols, outside): " << restoredImage.cols << std::endl;
        // std::cout << "Height (rows, outside): " << restoredImage.rows << std::endl;

        // restore: step 1
        restoreCroppedPatched(preprocessedImage, restoredImage, seqInfo);
        // cv::imwrite(taskInfo.outputPath, restoredImage);
        restoreFourCorners(preprocessedImage, restoredImage, seqInfo);
        cv::imwrite(taskInfo.outputPath, restoredImage);
    }
}

// todo: 与 preprocess 逻辑类似，如何复用？
void restoreCroppedPatched(const cv::Mat &srcImage, cv::Mat &dstImage,
                           const SequenceInfo &seqInfo) {
    int radius = static_cast<int>(seqInfo.diameter) / 2;
    int halfSideLength = std::floor(static_cast<double>(radius) / sqrt(2));
    int sideLength = 2 * halfSideLength + 1;

    // std::cout << "Width (cols): " << dstImage.cols << std::endl;
    // std::cout << "Height (rows): " << dstImage.rows << std::endl;

    for (int i = 0; i < seqInfo.centers.size(); i++) {
        cv::Point2i center(cvRound(seqInfo.centers[i].x), cvRound(seqInfo.centers[i].y));
        cv::Rect dstROI(center.x - halfSideLength, center.y - halfSideLength, sideLength,
                        sideLength);

        int srcX = (i % seqInfo.colNum) * sideLength;
        int srcY = (i / seqInfo.colNum) * sideLength;
        cv::Rect srcROI(srcX, srcY, sideLength, sideLength);
        cv::Mat currentPatch = srcImage(srcROI);

        // cv::Point ltop = dstROI.tl();
        // std::cout << "Top left of dstROI: " << ltop.x << ", " << ltop.y << std::endl;
        // cv::Point rbot = dstROI.br();
        // std::cout << "Bottom right of dstROI: " << rbot.x << ", " << rbot.y << std::endl;

        currentPatch.copyTo(dstImage(dstROI));
    }
}

// corner predict
std::array<cv::Mat, 4> getFourCornerMasks(const cv::Size &imageSize, cv::Point2i &center,
                                          int radius) {
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

void restoreFourCorners(const cv::Mat &srcImage, cv::Mat &dstImage, const SequenceInfo &seqInfo) {
    int radius = static_cast<int>(seqInfo.diameter) / 2;
    int halfSideLength = std::floor(static_cast<double>(radius) / sqrt(2));
    int sideLength = 2 * halfSideLength + 1;

    std::cout << "Restoring four corners..." << std::endl;

    std::array<cv::Point2i, 4> offsets = {cv::Point2i(0, 87), cv::Point2i(-76, -44),
                                          cv::Point2i(0, -87), cv::Point2i(76, -44)};

    for (int i = 0; i < seqInfo.centers.size(); i++) {
        // todo：暂时排除第一行、第一列、最后一行、最后一列
        if (i / seqInfo.colNum == 0 || i % seqInfo.colNum == 0 ||
            i / seqInfo.colNum == seqInfo.rowNum - 1 || i % seqInfo.colNum == seqInfo.colNum - 1)
            continue;

        if (i % 100 == 0) {
            std::cout << "Restoring micro image" << i << std::endl;
        }

        cv::Point2i center(cvRound(seqInfo.centers[i].x), cvRound(seqInfo.centers[i].y));
        std::array<cv::Mat, 4> fourCornersMasks =
            getFourCornerMasks(dstImage.size(), center, radius);

        for (int j = 0; j < 4; j++) {
            copyTo(dstImage, fourCornersMasks[j], offsets[j]);
        }
    }
}

void copyTo(cv::Mat &image, cv::Mat &dstMask, cv::Point2i &offset) {
    cv::Mat dstROI;
    image.copyTo(dstROI, dstMask);

    cv::Rect dstBoundingBox = cv::boundingRect(dstMask);
    cv::Rect srcBoundingBox = dstBoundingBox + offset;

    if (srcBoundingBox.x < 0 || srcBoundingBox.y < 0 ||
        srcBoundingBox.x + srcBoundingBox.width > image.cols ||
        srcBoundingBox.y + srcBoundingBox.height > image.rows) {
        std::cerr << "Error: srcROI is out of bounds." << std::endl;
        return;
    }

    cv::Mat srcROI = image(srcBoundingBox); // 从 image 中获取源区域

    srcROI.copyTo(image(dstBoundingBox), dstMask(dstBoundingBox));
}

} // namespace MCA2