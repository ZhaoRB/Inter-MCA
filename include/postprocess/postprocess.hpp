#pragma once

#include "data_structure.hpp"
#include <array>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <string>
#include <vector>

namespace MCA2 {
const int NeighborNum = 6;
const int CornerNum = 4;

class PostProcessor {
private:
    int radius, halfSideLength, sideLength;

    std::array<std::vector<cv::Point2i>, NeighborNum> offsetCandidates;

    // retore patches
    void restoreCroppedPatched(const cv::Mat &srcImage, cv::Mat &dstImage,
                               const SequenceInfo &seqInfo);

    // restore four corners
    void parseSupInfo(const std::string &supInfoPath);

    std::array<cv::Point2i, NeighborNum> findBestOffset(const cv::Mat &image,
                                                        const cv::Point2i &center);

    void restoreFourCorners(cv::Mat &dstImage, const SequenceInfo &seqInfo);

    std::array<cv::Mat, CornerNum> getFourCornerMasks(const cv::Size &imageSize,
                                                      const cv::Point2i &center);

    void copyTo(cv::Mat &image, cv::Mat &dstMask, const cv::Point2i &offset1,
                const cv::Point2i &offset2 = cv::Point2i(0, 0));

    void restoreCornersOfEdgeMI(cv::Mat &image, const cv::Point2i &center);

    void lumaCompensation(cv::Mat &image, const cv::Point2i &center);

    // restore gaps among MIs
    

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