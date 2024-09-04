#include "utils.hpp"

#include <iostream>
#include <opencv2/imgcodecs.hpp>

namespace MCA2 {
cv::Mat getInputImage(const std::string &pathPattern, int idx) {
    char filePath[256];
    std::snprintf(filePath, sizeof(filePath), pathPattern.c_str(), idx);
    cv::Mat image = cv::imread(filePath, cv::IMREAD_COLOR);

    if (image.empty()) {
        std::cerr << "Error: Unable to load image from " << filePath << std::endl;
    }

    return image;
}
} // namespace MCA2