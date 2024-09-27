#pragma once

#include "data-structure.hpp"
#include <array>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <vector>

namespace MCA2 {

void preprocess(SequenceInfo &seqInfo, TaskInfo &taskInfo);

void cropAndRealign(cv::Mat &rawImage, cv::Mat &croppedImage, cv::Point2i &center, int idx,
                    int colNum, int sideLength);



enum Direction { TOP, RTOP, RBOT, BOT, LBOT, LTOP };
const int NEIGHBOR_NUM = 6;

cv::Point2i calOffsetVector(const cv::Point2i &ltop, int w, int h, Direction direction);

std::array<cv::Point2i, NEIGHBOR_NUM>
calculateOffsetVectors(const cv::Mat &image, const std::vector<cv::Point2d> &centers, int idx,
                       int colNum, int rowNum, int diameter);
} // namespace MCA2