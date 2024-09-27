/*
Parse config file and calibration file
 */
#pragma once

#include <opencv2/core/types.hpp>
#include <string>

#include "data-structure.hpp"

namespace MCA2 {

class Parser {
private:
    void calAllCenterPoints(SequenceInfo &seqInfo);

    void setRowAndColNums(SequenceInfo &seqInfo);

public:
    int parseConfigFile(std::string &cfgFilePath, TaskInfo &taskInfo);

    int parseCalibXMLFile(std::string &calibXMLFilePath, SequenceInfo &seqInfo);
};

} // namespace MCA2