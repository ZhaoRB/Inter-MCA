#include "preprocess.hpp"
#include "data_structure.hpp"
#include "utils.hpp"

#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <ostream>
#include <string>

namespace MCA2 {

void PreProcessor::preprocess(SequenceInfo &seqInfo, TaskInfo &taskInfo) {
    // calculate offset vector candidates through first frame
    cv::Mat firstFrame = cv::imread(getPath(taskInfo.inputPath, 0));
    calOffsetVectors(firstFrame, seqInfo);

    for (int i = taskInfo.startFrame; i <= taskInfo.endFrame; i++) {
        // read image
        std::string inputRealPath = getPath(taskInfo.inputPath, i);
        cv::Mat image = cv::imread(inputRealPath);
        if (image.empty()) {
            std::cout << "[Warning] Image not found: " << inputRealPath << std::endl;
            continue;
        }

        // crop and realign
        cv::Mat processedImage = cropAndRealign(image, seqInfo);
        cv::imwrite(getPath(taskInfo.outputPath, i), processedImage);
    }
};

} // namespace MCA2