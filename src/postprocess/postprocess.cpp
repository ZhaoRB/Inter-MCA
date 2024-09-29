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

        // todo: width 和 height 存在哪里？哪个对象中？
        cv::Mat restoredImage = cv::Mat::zeros(cv::Size(seqInfo.width, seqInfo.height), CV_8UC3);
        // std::cout << "Width (cols, outside): " << restoredImage.cols << std::endl;
        // std::cout << "Height (rows, outside): " << restoredImage.rows << std::endl;

        // restore: step 1
        restoreCroppedPatched(preprocessedImage, restoredImage, seqInfo);
        // cv::imwrite(taskInfo.outputPath, restoredImage);
        restoreFourCorners(preprocessedImage, restoredImage, seqInfo);
        cv::imwrite(taskInfo.outputPath, restoredImage);
    }
}

} // namespace MCA2