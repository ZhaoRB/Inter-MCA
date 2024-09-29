#include "data_structure.hpp"
#include "preprocess.hpp"

#include <opencv2/core.hpp>

namespace MCA2 {

cv::Mat Preprocessor::cropAndRealign(const cv::Mat &rawImage, const SequenceInfo &seqInfo) {
    cv::Mat processedImage;

    return processedImage;
}

// void Preprocessor::cropAndRealign(cv::Mat &rawImage, cv::Mat &croppedImage, cv::Point2i &center,
//                                   int idx, int colNum, int sideLength) {
//     int halfSideLength = sideLength / 2;
//     cv::Rect roiRect(center.x - halfSideLength, center.y - halfSideLength, sideLength,
//     sideLength); cv::Mat croppedMicroImage = rawImage(roiRect);

//     int targetX = (idx % colNum) * sideLength;
//     int targetY = (idx / colNum) * sideLength;
//     cv::Rect targetROI(targetX, targetY, sideLength, sideLength);
//     croppedMicroImage.copyTo(croppedImage(targetROI));
// }

} // namespace MCA2