#pragma once

#include <opencv2/core/mat.hpp>
#include <string>

namespace MCA2 {
cv::Mat getInputImage(const std::string &pathPattern, int idx);

double calculateSSIM(cv::Mat &img1, cv::Mat &img2);

} // namespace MCA2