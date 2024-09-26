#include "preprocess.hpp"
#include "data-structure.hpp"
#include "utils.hpp"

#include <array>
#include <cmath>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/fast_math.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/operations.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <ostream>
#include <string>

namespace MCA2 {
/**

*/
void preprocess(SequenceInfo &seqInfo, TaskInfo &taskInfo) {
    int radius = static_cast<int>(seqInfo.diameter) / 2;
    int halfSideLength = std::floor(static_cast<double>(radius) / sqrt(2));
    int sideLength = 2 * halfSideLength + 1;
    bool hasCalculateOffsetVector = false;

    for (int i = taskInfo.startFrame; i <= taskInfo.endFrame; i++) {
        // read image
        cv::Mat image = getInputImage(taskInfo.inputPath, i);
        if (image.empty()) {
            std::cout << "[Warning] Image not found: " << taskInfo.inputPath << std::endl;
            continue;
        }

        // calculate offset vector
        if (!hasCalculateOffsetVector) {
            auto offsetVectors =
                calculateOffsetVectors(image, seqInfo.centers, 1430, seqInfo.colNum, seqInfo.rowNum,
                                       static_cast<int>(seqInfo.diameter));
            for (auto &offset : offsetVectors) {
                std::cout << offset.x << ", " << offset.y << std::endl;
            }
        }

        // crop and realign
        cv::Mat croppedImage = cv::Mat::zeros(
            cv::Size(seqInfo.colNum * sideLength, seqInfo.rowNum * sideLength), CV_8UC3);

        for (int idx = 0; idx < seqInfo.centers.size(); idx++) {
            cv::Point2i center(cvRound(seqInfo.centers[idx].x), cvRound(seqInfo.centers[idx].y));
            cropAndRealign(image, croppedImage, center, idx, seqInfo.colNum, sideLength);
        }

        // todo: 这个 filename 应该还得改一下
        cv::imwrite(taskInfo.outputPath, croppedImage);
    }
};

// MCA
void cropAndRealign(cv::Mat &rawImage, cv::Mat &croppedImage, cv::Point2i &center, int idx,
                    int colNum, int sideLength) {
    int halfSideLength = sideLength / 2;
    cv::Rect roiRect(center.x - halfSideLength, center.y - halfSideLength, sideLength, sideLength);
    cv::Mat croppedMicroImage = rawImage(roiRect);

    int targetX = (idx % colNum) * sideLength;
    int targetY = (idx / colNum) * sideLength;
    cv::Rect targetROI(targetX, targetY, sideLength, sideLength);
    croppedMicroImage.copyTo(croppedImage(targetROI));
}

std::array<cv::Point2i, NEIGHBOR_NUM>
calculateOffsetVectors(const cv::Mat &image, const std::vector<cv::Point2d> &centers, int idx,
                       int colNum, int rowNu, int diameter) {
    std::array<cv::Point2i, NEIGHBOR_NUM> offsetVectors;
    std::array<double, NEIGHBOR_NUM> ssimScores = {}; // init as 0

    for (auto ssim : ssimScores) {
        std::cout << ssim << std::endl;
    }

    cv::Point2i curCenter(std::round(centers[idx].x), std::round(centers[idx].y));
    int radius = diameter / 2;
    int halfSideLength = static_cast<int>(radius / sqrt(2));
    int sideLength = halfSideLength * 2 + 1;

    std::array<cv::Rect, NEIGHBOR_NUM> roiRects = {
        cv::Rect(curCenter.x - halfSideLength, curCenter.y + 1, sideLength, halfSideLength),
        cv::Rect(curCenter.x - halfSideLength, curCenter.y + 1, halfSideLength, halfSideLength),
        cv::Rect(curCenter.x - halfSideLength, curCenter.y - halfSideLength, halfSideLength,
                 halfSideLength),
        cv::Rect(curCenter.x - halfSideLength, curCenter.y - halfSideLength, sideLength,
                 halfSideLength),
        cv::Rect(curCenter.x + 1, curCenter.y - halfSideLength, halfSideLength, halfSideLength),
        cv::Rect(curCenter.x + 1, curCenter.y + 1, halfSideLength, halfSideLength)};

    auto findBestOffset = [&](int idx, int startY, int endY, int signX, int signY, double factor) {
        int test = 0;
        for (int biasY = startY; biasY <= endY; biasY++) {
            cv::Point2i offset(signX * static_cast<int>(factor * biasY), signY * biasY);
            cv::Rect targetROI = roiRects[idx] + offset;
            double ssim = calculateSSIM(image(roiRects[idx]), image(targetROI));
            if (ssim > ssimScores[idx]) {
                test++;
                ssimScores[idx] = ssim;
                offsetVectors[idx] = offset;
            }
        }
        std::cout << "ssim score: " << ssimScores[idx] << " , test number: " << test << std::endl;
    };

    int rangeY1[] = {2 * radius, 2 * radius + halfSideLength};
    findBestOffset(0, rangeY1[0], rangeY1[1], 0, -1, 1.0); // TOP
    findBestOffset(3, rangeY1[0], rangeY1[1], 0, 1, 1.0);  // BOT

    int rangeY2[] = {radius, static_cast<int>(1.5 * radius)};
    findBestOffset(1, rangeY2[0], rangeY2[1], 1, -1, sqrt(3));  // RTOP
    findBestOffset(2, rangeY2[0], rangeY2[1], 1, 1, sqrt(3));   // RBOT
    findBestOffset(4, rangeY2[0], rangeY2[1], -1, 1, sqrt(3));  // LBOT
    findBestOffset(5, rangeY2[0], rangeY2[1], -1, -1, sqrt(3)); // LTOP

    cv::imwrite(
        "/Users/riverzhao/Project/Codec/0_lvc_codec/Inter-MCA/data/temp/matchRegion/patch.png",
        image(cv::Rect(curCenter.x - halfSideLength, curCenter.y - halfSideLength, sideLength,
                       sideLength)));

    for (int i = 0; i < NEIGHBOR_NUM; i++) {
        auto img1 = image(roiRects[i]);
        auto img2 = image(roiRects[i] + offsetVectors[i]);

        std::string outputPath =
            "/Users/riverzhao/Project/Codec/0_lvc_codec/Inter-MCA/data/temp/matchRegion/raw-pos";
        outputPath = outputPath + std::to_string(i) + ".png";
        cv::imwrite(outputPath, img1);

        size_t pos = outputPath.find("raw");
        if (pos != std::string::npos) {
            outputPath.replace(pos, 3, "predict");
        }
        cv::imwrite(outputPath, img2);
    }

    return offsetVectors;
}
} // namespace MCA2