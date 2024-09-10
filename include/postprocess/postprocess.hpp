#pragma once

#include "data-structure.hpp"
#include <opencv2/core/mat.hpp>
// #include <string>

namespace MCA2 {
void postprocess(SequenceInfo &seqInfo, TaskInfo &taskInfo);

void restoreCroppedPatched(const cv::Mat &srcImage, cv::Mat &dstImage, const SequenceInfo &seqInfo);

// four corners = micro image - cropped patch
std::array<cv::Mat, 4> getFourCornerMasks(const cv::Size &imageSize, cv::Point2i &center,
                                          int radius);

void restoreFourCorners(const cv::Mat &srcImage, cv::Mat &dstImage, const SequenceInfo &seqInfo);

void copyTo(cv::Mat &image, cv::Mat &dstMask, cv::Point2i &offset);
} // namespace MCA2