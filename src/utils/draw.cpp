#include "parse.hpp"
#include <iostream>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <string>

namespace MCA2 {
int drawCenters(SequenceInfo seq, std::string &imagePath, std::string &outputPath) {
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);

    if (image.empty()) {
        std::cerr << "Can't read image " << imagePath << std::endl;
        return -1;
    }

    cv::Mat image_copy = image.clone();

    for (const auto &point : seq.centers) {
        cv::Point center(cvRound(point.x), cvRound(point.y));
        cv::Scalar color(0, 0, 255);
        cv::circle(image_copy, center, int(seq.diameter / 2), color, 2);
    }

    cv::imwrite(outputPath, image_copy);

    return 0;
}
} // namespace MCA2