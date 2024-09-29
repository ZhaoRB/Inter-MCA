#pragma once

#include "data_structure.hpp"
#include <opencv2/core/mat.hpp>

namespace MCA2 {
class PostProcessor {
private:
    int radius, halfSideLength, sideLength;

    // retore main part
    void restoreCroppedPatched(const cv::Mat &srcImage, cv::Mat &dstImage,
                               const SequenceInfo &seqInfo);

    void restoreFourCorners(const cv::Mat &srcImage, cv::Mat &dstImage,
                            const SequenceInfo &seqInfo);

    std::array<cv::Mat, 4> getFourCornerMasks(const cv::Size &imageSize, cv::Point2i &center);

    void copyTo(cv::Mat &image, cv::Mat &dstMask, cv::Point2i &offset);

    // restore edge
    void restoreEdge();


public:
    void postprocess(SequenceInfo &seqInfo, TaskInfo &taskInfo);

    PostProcessor() {};
    PostProcessor(double diameter) {
        radius = static_cast<int>(diameter) / 2;
        halfSideLength = std::floor(static_cast<double>(radius) / sqrt(2));
        sideLength = 2 * halfSideLength + 1;
    };
};
} // namespace MCA2