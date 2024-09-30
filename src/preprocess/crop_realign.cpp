#include "data_structure.hpp"
#include "preprocess.hpp"

#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>

namespace MCA2 {

cv::Size PreProcessor::calProcessedSize(const SequenceInfo &seqInfo) {
    int width = sideLength * seqInfo.colNum;
    int height = sideLength * seqInfo.rowNum;

    int error = -5;
    if ((seqInfo.height - seqInfo.lbot.y - seqInfo.diameter) > error) {
        height = height + halfSideLength;
    }

    return cv::Size(width, height);
}

cv::Mat PreProcessor::cropAndRealign(const cv::Mat &rawImage, const SequenceInfo &seqInfo) {
    cv::Mat processedImage(calProcessedSize(seqInfo), CV_8UC3);

    // todo: 有点硬编码，待优化
    for (int i = 0; i < seqInfo.colNum; i++) {
        for (int j = 0; j < seqInfo.rowNum; j++) {
            if (j == seqInfo.rowNum - 1 && seqInfo.colNum % 2 == 1 && i % 2 == 1)
                continue;

            cv::Point2i curCenter(std::round(seqInfo.centers[i * seqInfo.rowNum + j].x),
                                  std::round(seqInfo.centers[i * seqInfo.rowNum + j].y));
            int srcX = curCenter.x - halfSideLength;
            int srcY = curCenter.y - halfSideLength;
            int tgtX = i * sideLength;
            int tgtY = i % 2 == 0 ? j * sideLength : j * sideLength + halfSideLength;

            rawImage(cv::Rect(srcX, srcY, sideLength, sideLength))
                .copyTo(processedImage(cv::Rect(tgtX, tgtY, sideLength, sideLength)));
        }

        if (i % 2 == 1) {
            cv::Point2i curCenter(
                std::round(seqInfo.centers[i * seqInfo.rowNum].x),
                std::round(seqInfo.centers[i * seqInfo.rowNum].y - seqInfo.diameter));
            int srcX = curCenter.x - halfSideLength;
            int srcY = curCenter.y;
            int tgtX = i * sideLength, tgtY = 0;
            rawImage(cv::Rect(srcX, srcY, sideLength, halfSideLength))
                .copyTo(processedImage(cv::Rect(tgtX, tgtY, sideLength, halfSideLength)));
        }
        if (i % 2 == 1 && seqInfo.colNum % 2 == 1 || i % 2 == 0 && seqInfo.colNum % 2 == 0) {
            int biasY = seqInfo.colNum % 2 == 1 ? 0 : seqInfo.diameter;
            cv::Point2i curCenter(
                std::round(seqInfo.centers[i * seqInfo.rowNum + seqInfo.rowNum - 1].x),
                std::round(seqInfo.centers[i * seqInfo.rowNum + seqInfo.rowNum - 1].y) + biasY);
            int srcX = curCenter.x - halfSideLength;
            int srcY = curCenter.y - halfSideLength;
            int tgtX = i * sideLength;
            int tgtY = seqInfo.colNum % 2 == 0 ? seqInfo.rowNum * sideLength
                                               : (seqInfo.rowNum - 1) * sideLength + halfSideLength;
            rawImage(cv::Rect(srcX, srcY, sideLength, halfSideLength))
                .copyTo(processedImage(cv::Rect(tgtX, tgtY, sideLength, halfSideLength)));
        }
    }

    return processedImage;
}

} // namespace MCA2