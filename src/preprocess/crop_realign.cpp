#include "data_structure.hpp"
#include "preprocess.hpp"

#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>

namespace MCA2 {

/**
1. 裁切边缘像素 & 边缘MI
2. 中间非边缘MI，按照以前的方法，裁剪，拼接
 */
cv::Mat PreProcessor::cropAndRealign(const cv::Mat &rawImage, const SequenceInfo &seqInfo) {
    cv::Mat processedImage = cropAndRealignMainPart(rawImage, seqInfo);

    return processedImage;
}

cv::Mat PreProcessor::cropAndRealignMainPart(const cv::Mat &rawImage, const SequenceInfo &seqInfo) {
    cv::Mat processedImageMainPart(
        cv::Size(sideLength * (seqInfo.colNum - 2), sideLength * (seqInfo.rowNum - 2)), CV_8UC3);

    for (int i = 1; i < seqInfo.colNum - 1; i++) {
        for (int j = 1; j < seqInfo.rowNum - 1; j++) {
            cv::Point2i curCenter(std::round(seqInfo.centers[i * seqInfo.rowNum + j].x),
                                  std::round(seqInfo.centers[i * seqInfo.rowNum + j].y));

            cv::Rect srcROI(curCenter.x - halfSideLength, curCenter.y - halfSideLength, sideLength,
                            sideLength);
            cv::Mat croppedMicroImage = rawImage(srcROI);

            int targetX = (i - 1) * sideLength;
            int targetY = (j - 1) * sideLength;
            cv::Rect targetROI(targetX, targetY, sideLength, sideLength);
            croppedMicroImage.copyTo(processedImageMainPart(targetROI));
        }
    }

    return processedImageMainPart;
}

} // namespace MCA2