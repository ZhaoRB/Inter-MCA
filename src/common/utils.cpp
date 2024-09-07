#include "utils.hpp"

#include <opencv2/imgcodecs.hpp>

namespace MCA2 {
cv::Mat getInputImage(const std::string &pathPattern, int idx) {
    char filePath[256];
    std::snprintf(filePath, sizeof(filePath), pathPattern.c_str(), idx);
    cv::Mat image = cv::imread(filePath, cv::IMREAD_COLOR);
    return image;
}
} // namespace MCA2