#pragma once

#include "data-structure.hpp"
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <vector>

namespace MCA2 {
// v1: no parallelism, fixed crop size
void preprocess(const SequenceInfo &seqInfo, const TaskInfo &taskInfo);

void cropAndRealign(cv::Mat &image, cv::Mat &croppedImage, const SequenceInfo &seqInfo, int idx,
                    int sideLength);

// void getPredictVector(cv::Mat);

// temp function
void analysis(const cv::Mat &image, const std::vector<cv::Point2d> &centers, double diameter);
} // namespace MCA2