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

    std::array<cv::Point2i, 4> offsets = {cv::Point2i(0, 90), cv::Point2i(-77, -45),
                                          cv::Point2i(0, -90), cv::Point2i(77, -45)};

    for (int i = 0; i < seqInfo.centers.size(); i++) {
        // todo：暂时排除第一行、第一列、最后一行、最后一列
        if (i / seqInfo.colNum == 0 || i % seqInfo.colNum == 0 ||
            i / seqInfo.colNum == seqInfo.rowNum - 1 || i % seqInfo.colNum == seqInfo.colNum - 1)
            continue;

        cv::Point2i center(cvRound(seqInfo.centers[i].x), cvRound(seqInfo.centers[i].y));
        std::array<cv::Mat, 4> fourCornersMasks =
            getFourCornerMasks(dstImage.size(), center, radius);

        cv::Rect dstMaskROI(0, 90, dstImage.cols, dstImage.rows - 90);
        cv::Rect srcMaskROI(0, 90, dstImage.cols, dstImage.rows - 90);
    }
}

// 用 dstMask 和 offset 计算 srcMask，将 src copy 到 dst
void copyTo(cv::Mat &image, cv::Mat &dstMask, cv::Point2i &offset) {
    int rectWidth = image.cols - abs(offset.x), rectHeight = image.rows - abs(offset.y);
    cv::Point2i ltopSrc, ltopDst;

    if (offset.x > 0) {
        ltopDst.x = 0;
        ltopSrc.x = offset.x;
    } else {
        ltopDst.x = -1 * offset.x;
        ltopSrc.x = 0;
    }
    if (offset.y > 0) {
        ltopDst.y = 0;
        ltopSrc.y = offset.y;
    } else {
        ltopDst.y = -1 * offset.y;
        ltopSrc.y = 0;
    }
    cv::Rect roiSrc(ltopSrc.x, ltopSrc.y, rectWidth, rectHeight);
    cv::Rect roiDst(ltopDst.x, ltopDst.y, rectWidth, rectHeight);

    cv::Mat srcMask = cv::Mat::zeros(dstMask.size(), CV_8UC1);
    dstMask(roiDst).copyTo(srcMask(roiSrc));
}
} // namespace MCA2