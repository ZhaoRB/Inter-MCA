#pragma once

#include <opencv2/core/mat.hpp>
#include <string>

namespace MCA2 {

double calculateSSIM(const cv::Mat &img1, const cv::Mat &img2);

bool hasFormatSpecifier(const std::string &str);

std::string getPath(const std::string &pathPattern, int idx);

} // namespace MCA2