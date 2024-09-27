#pragma once

#include "data-structure.hpp"
#include <array>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <vector>

namespace MCA2 {
const int NeighborNum = 6; // each MI has 6 neighbors
class Preprocessor {
private:
    int radius, halfSideLength, sideLength;

    // additional data pass to postprocessor
    std::array<cv::Point2i, NeighborNum> offsets;
    std::vector<double> decayCoefficients;

    void cropAndRealign(cv::Mat &rawImage, cv::Mat &croppedImage, cv::Point2i &center, int idx,
                        int colNum, int sideLength);

    void calOffsetVectors(const cv::Mat &image, SequenceInfo &seqInfo);

    void calOffsetVectorsFromOneMI(const cv::Mat &image, const cv::Point2i &curCenter,
                                   std::array<double, NeighborNum> &ssimScores,
                                   std::array<cv::Point2i, NeighborNum> &tmpOffsets);

public:
    Preprocessor() {};
    Preprocessor(double diameter) {
        radius = static_cast<int>(diameter) / 2;
        halfSideLength = std::floor(static_cast<double>(radius) / sqrt(2));
        sideLength = 2 * halfSideLength + 1;
    };

    void preprocess(SequenceInfo &seqInfo, TaskInfo &taskInfo);
};

} // namespace MCA2