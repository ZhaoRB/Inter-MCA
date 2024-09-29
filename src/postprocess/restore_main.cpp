#include "postprocess.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <opencv2/imgproc.hpp>

namespace MCA2 {

//TODO: reuse with preprocessor
void PostProcessor::restoreCroppedPatched(const cv::Mat &processedImage, cv::Mat &restoredImage,
                                          const SequenceInfo &seqInfo) {

    for (int i = 1; i < seqInfo.colNum - 1; i++) {
        for (int j = 1; j < seqInfo.rowNum - 1; j++) {
            cv::Point2i curCenter(std::round(seqInfo.centers[i * seqInfo.rowNum + j].x),
                                  std::round(seqInfo.centers[i * seqInfo.rowNum + j].y));

            cv::Rect restoredROI(curCenter.x - halfSideLength, curCenter.y - halfSideLength, sideLength,
                            sideLength);
            cv::Mat restoredPatch = restoredImage(restoredImage);

            int srcX = (i - 1) * sideLength;
            int srcY = (j - 1) * sideLength;
            cv::Rect srcROI(srcX, srcY, sideLength, sideLength);
            cv::Mat srcPatch = processedImage(srcROI);

            srcPatch.copyTo(restoredPatch);
        }
    }
}

// corner predict
std::array<cv::Mat, 4> PostProcessor::getFourCornerMasks(const cv::Size &imageSize,
                                                         cv::Point2i &center) {

    std::array<cv::Mat, 4> fourCornersMasks;
    for (auto &mask : fourCornersMasks) {
        mask = cv::Mat::zeros(imageSize, CV_8UC1);
        cv::circle(mask, center, radius, cv::Scalar(255), -1);
    }

    std::array<cv::Rect, 4> rectMasks;
    int longEdge = radius * 2 + 1, shortEdge = halfSideLength + radius + 1;
    rectMasks[0] = cv::Rect(center.x - radius, center.y - halfSideLength, longEdge, shortEdge);
    rectMasks[1] = cv::Rect(center.x - radius, center.y - radius, shortEdge, longEdge);
    rectMasks[2] = cv::Rect(center.x - radius, center.y - radius, longEdge, shortEdge);
    rectMasks[3] = cv::Rect(center.x - halfSideLength, center.y - radius, shortEdge, longEdge);
    for (int i = 0; i < 4; i++) {
        cv::rectangle(fourCornersMasks[i], rectMasks[i], cv::Scalar(0), -1);
    }

    return fourCornersMasks;
}

void PostProcessor::restoreFourCorners(const cv::Mat &srcImage, cv::Mat &dstImage,
                                       const SequenceInfo &seqInfo) {
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
        std::array<cv::Mat, 4> fourCornersMasks = getFourCornerMasks(dstImage.size(), center);

        for (int j = 0; j < 4; j++) {
            copyTo(dstImage, fourCornersMasks[j], offsets[j]);
        }
    }
}

void PostProcessor::copyTo(cv::Mat &image, cv::Mat &dstMask, cv::Point2i &offset) {
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