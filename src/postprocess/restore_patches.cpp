#include "postprocess.hpp"
#include <cmath>
#include <cstdlib>

namespace MCA2 {

// TODO: reuse with preprocessor
void PostProcessor::restoreCroppedPatched(const cv::Mat &processedImage, cv::Mat &restoredImage,
                                          const SequenceInfo &seqInfo) {
    for (int i = 0; i < seqInfo.colNum; i++) {
        for (int j = 0; j < seqInfo.rowNum; j++) {
            if (j == seqInfo.rowNum - 1 && seqInfo.colNum % 2 == 1 && i % 2 == 1)
                continue;

            cv::Point2i curCenter(std::round(seqInfo.centers[i * seqInfo.rowNum + j].x),
                                  std::round(seqInfo.centers[i * seqInfo.rowNum + j].y));

            int srcX = curCenter.x - halfSideLength;
            int srcY = curCenter.y - halfSideLength;
            int tgtX = i * sideLength;
            int tgtY = i % 2 == 0 ? j * sideLength : j * sideLength + halfSideLength;

            processedImage(cv::Rect(tgtX, tgtY, sideLength, sideLength))
                .copyTo(restoredImage(cv::Rect(srcX, srcY, sideLength, sideLength)));
        }

        if (i % 2 == 1) {
            cv::Point2i curCenter(
                std::round(seqInfo.centers[i * seqInfo.rowNum].x),
                std::round(seqInfo.centers[i * seqInfo.rowNum].y - seqInfo.diameter));

            int srcX = curCenter.x - halfSideLength;
            int srcY = curCenter.y;
            int tgtX = i * sideLength;
            int tgtY = 0;

            processedImage(cv::Rect(tgtX, tgtY, sideLength, halfSideLength))
                .copyTo(restoredImage(cv::Rect(srcX, srcY, sideLength, halfSideLength)));
        }
        if (i % 2 == 1 && seqInfo.colNum % 2 == 1 || i % 2 == 0 && seqInfo.colNum % 2 == 0) {
            int biasY = seqInfo.colNum % 2 == 1 ? 0 : seqInfo.diameter;
            cv::Point2i curCenter(
                std::round(seqInfo.centers[i * seqInfo.rowNum + seqInfo.rowNum - 1].x),
                std::round(seqInfo.centers[i * seqInfo.rowNum + seqInfo.rowNum - 1].y) + biasY);

            int srcX = curCenter.x - halfSideLength;
            int srcY = curCenter.y - halfSideLength;
            int tgtX = i * sideLength;
            int tgtY = seqInfo.colNum % 2 == 0 ? seqInfo.rowNum * sideLength
                                               : (seqInfo.rowNum - 1) * sideLength + halfSideLength;
            processedImage(cv::Rect(tgtX, tgtY, sideLength, halfSideLength))
                .copyTo(restoredImage(cv::Rect(srcX, srcY, sideLength, halfSideLength)));
        }
    }
}

} // namespace MCA2