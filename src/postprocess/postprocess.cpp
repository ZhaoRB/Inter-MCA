#include "postprocess.hpp"
#include "utils.hpp"
#include <iostream>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/types.hpp>
#include <opencv2/imgcodecs.hpp>

namespace MCA2 {
void postprocess(SequenceInfo &seqInfo, TaskInfo &taskInfo) {
    for (int i = taskInfo.startFrame; i <= taskInfo.endFrame; i++) {
        // read image
        cv::Mat preprocessedImage = getInputImage(taskInfo.inputPath, i);
        if (preprocessedImage.empty()) {
            std::cout << "[Warning] Image not found: " << taskInfo.inputPath << std::endl;
            continue;
        }

        cv::Mat restoredImage = cv::Mat::zeros(cv::Size(seqInfo.width, seqInfo.height), CV_8UC3);

        // restore: step 1
        restoreCroppedPatched(preprocessedImage, restoredImage, seqInfo);
        cv::imwrite(taskInfo.outputPath, restoredImage);
    }
}

// todo: 与 preprocess 逻辑类似，如何复用？
void restoreCroppedPatched(const cv::Mat &srcImage, cv::Mat &dstImage, const SequenceInfo &seqInfo) {
    int radius = static_cast<int>(seqInfo.diameter) / 2;
    int halfSideLength = std::floor(static_cast<double>(radius) / sqrt(2));
    int sideLength = 2 * halfSideLength + 1;

    for (int i = 0; i < seqInfo.centers.size(); i++) {
        cv::Point2i center(cvRound(seqInfo.centers[i].x), cvRound(seqInfo.centers[i].y));
        cv::Rect dstROI(center.x - halfSideLength, center.y - halfSideLength, sideLength,
                        sideLength);

        int targetX = (i % seqInfo.colNum) * sideLength;
        int targetY = (i / seqInfo.colNum) * sideLength;
        cv::Rect srcROI(targetX, targetY, sideLength, sideLength);

        srcImage(srcROI).copyTo(dstImage(srcROI));
    }
}
} // namespace MCA2