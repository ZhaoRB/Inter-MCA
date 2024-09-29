#pragma once

#include "data_structure.hpp"
#include <array>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <string>
#include <vector>

namespace MCA2 {
const int NeighborNum = 6; // each MI has 6 neighbors
class Preprocessor {
private:
    int radius, halfSideLength, sideLength;

    // offset vectors
    std::array<std::vector<std::pair<std::string, int>>, NeighborNum> offsetsCandidates;

    void calOffsetVectors(const cv::Mat &image, SequenceInfo &seqInfo);
    void calOffsetVectorsFromOneMI(const cv::Mat &image, const cv::Point2i &curCenter,
                                   std::array<double, NeighborNum> &ssimScores,
                                   std::array<cv::Point2i, NeighborNum> &tmpOffsets);
    void saveOffsetVectors(std::string &supInfoPath);

    //
    std::vector<double> decayCoefficients;

    // crop and realign
    cv::Mat cropAndRealign(const cv::Mat &rawImage, const SequenceInfo &seqInfo);
    cv::Mat cropAndRealignMainPart(const cv::Mat &rawImage, const SequenceInfo &seqInfo);

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