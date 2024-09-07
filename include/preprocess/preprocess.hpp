#pragma once

#include "data-structure.hpp"
#include <array>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <vector>

namespace MCA2 {
// v1: no parallelism, fixed crop size
void preprocess(SequenceInfo &seqInfo, TaskInfo &taskInfo);

void cropAndRealign(cv::Mat &rawImage, cv::Mat &croppedImage, cv::Point2i &center, int idx,
                    int colNum, int sideLength);

// four corners = micro image - cropped patch
std::array<cv::Mat, 4> getFourCorners(const cv::Size &imageSize, cv::Point2i &center, int radius);

cv::Point2i calOffsetVector();

// temp function
void analysis(const cv::Mat &image, const std::vector<cv::Point2d> &centers, int diameter);
void analysis2();
} // namespace MCA2