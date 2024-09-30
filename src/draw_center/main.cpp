#include <iostream>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "data_structure.hpp"
#include "parse.hpp"

void drawCenters(const std::vector<cv::Point2d> &centers, double diameter, std::string &imagePath,
                 std::string &outputPath) {
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);

    if (image.empty()) {
        std::cerr << "Can't read image " << imagePath << std::endl;
        return;
    }

    cv::Mat image_copy = image.clone();

    for (const auto &point : centers) {
        cv::Point center(cvRound(point.x), cvRound(point.y));
        cv::Scalar color(0, 0, 255);
        cv::circle(image_copy, center, static_cast<int>(diameter / 2), color, 2);
    }

    cv::imwrite(outputPath, image_copy);
}

int main(int argc, char **argv) {
    std::string calibFilePath =
        "/Users/riverzhao/Project/Codec/0_lvc_codec/Inter-MCA/config/TSPC/Boys/mca_calib.xml";
    std::string inputPath =
        "/Users/riverzhao/Project/Codec/0_lvc_codec/Inter-MCA/data/raw/processed/Boys.png";
    std::string outputPath =
        "/Users/riverzhao/Project/Codec/0_lvc_codec/Inter-MCA/data/center/Boys.png";

    MCA2::SequenceInfo seqInfo;

    MCA2::Parser parser;
    parser.parseCalibXMLFile(calibFilePath, seqInfo);

    drawCenters(seqInfo.centers, seqInfo.diameter, inputPath, outputPath);

    return 0;
}