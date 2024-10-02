#pragma once

#include <opencv2/core/mat.hpp>
#include <string>

namespace MCA2 {

double calculateSSIM(const cv::Mat &img1, const cv::Mat &img2);

bool hasFormatSpecifier(const std::string &str);

std::string getPath(const std::string &pathPattern, int idx);

cv::Point stringToPoint(const std::string& str);

std::string pointToString(const cv::Point& point);

double calculateDistance(const cv::Point &point1, const cv::Point &point2);

cv::Mat expandImage(const cv::Mat &srcImage, int addRow, int addCol);

cv::Mat cropImage(const cv::Mat &srcImage, int uselessRow, int uselessCol);

} // namespace MCA2