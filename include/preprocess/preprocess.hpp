#pragma once

#include "data-structure.hpp"
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>

namespace MCA2 {
// v1: no parallelism, fixed crop size
void preprocess(SequenceInfo &seqInfo, TaskInfo &taskInfo);

// Get one image object from input path pattern and frame number
cv::Mat getInputImage(std::string &pathPattern, int idx);

double calculateSSIM(cv::Mat &img1, cv::Mat &img2);

// cv::Mat extractMicroImage(cv::Mat &image, const cv::Point2d &center, double diameter);

//  
cv::Mat crop(cv::Mat &image, cv::Point2d &center, int diameter, int index);

} // namespace MCA2