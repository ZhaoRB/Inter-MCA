#pragma once

#include <opencv2/core/mat.hpp>
#include <string>

namespace MCA2 {
cv::Mat getInputImage(const std::string &pathPattern, int idx);

double calculateSSIM(const cv::Mat &img1, const cv::Mat &img2);

bool hasFormatSpecifier(const std::string &str);

} // namespace MCA2