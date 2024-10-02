#include "postprocess.hpp"
#include "utils.hpp"
#include <cstdlib>
#include <iostream>
#include <opencv2/imgcodecs.hpp>

namespace MCA2 {
void PostProcessor::postprocess(SequenceInfo &seqInfo, TaskInfo &taskInfo) {
    for (int i = taskInfo.startFrame; i <= taskInfo.endFrame; i++) {
        // read image
        cv::Mat preprocessedImage = cv::imread(getPath(taskInfo.inputPath, i));
        if (preprocessedImage.empty()) {
            std::cout << "[Warning] Image not found: " << taskInfo.inputPath << std::endl;
            continue;
        }

        cv::Mat restoredImage = cv::Mat::zeros(cv::Size(seqInfo.width, seqInfo.height), CV_8UC3);

        if (seqInfo.colNum % 2 == 1) {
            preprocessedImage = expandImage(preprocessedImage, halfSideLength, sideLength);
            restoredImage = expandImage(restoredImage, radius, std::round(sqrt(3) * radius));
        }

        // restore patches
        restoreCroppedPatched(preprocessedImage, restoredImage, seqInfo);

        // restore four corners
        // parseSupInfo(taskInfo.supInfoPath);
        // restoreFourCorners(restoredImage, seqInfo);

        if (seqInfo.colNum % 2 == 1) {
            restoredImage = cropImage(restoredImage, radius, std::round(sqrt(3) * radius));
        }

        cv::imwrite(getPath(taskInfo.outputPath, i), restoredImage);
    }
}

} // namespace MCA2