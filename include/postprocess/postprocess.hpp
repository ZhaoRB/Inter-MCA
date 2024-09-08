#pragma once

#include "data-structure.hpp"
#include <opencv2/core/mat.hpp>
// #include <string>

namespace MCA2 {
class PostProcessor {
private:
    cv::Mat croppedImage;
    cv::Mat restoredImage;
    SequenceInfo sequenceInfo;

    void restorePatches();
    void restoreCorners();

public:
    cv::Mat postprocess();

    // PostProcessor(const std::string &croppedImagePaht, const std::string &restoredImagePath);
};
} // namespace MCA2