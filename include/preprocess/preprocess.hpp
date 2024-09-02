#pragma once

#include "data-structure.hpp"
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>

namespace MCA2 {
void preprocess(SequenceInfo &seqInfo, TaskInfo &taskInfo);

cv::Mat getInputImage(std::string &pathPattern, int idx);

cv::Mat extractMicroImage(cv::Mat &image, cv::Point2d &center, int diameter);

cv::Mat crop(cv::Mat &microImage, cv::Point2d &center, int diameter);

double calculateSSIM(cv::Mat &img1, cv::Mat &img2);

} // namespace MCA2