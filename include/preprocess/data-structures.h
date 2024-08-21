# include <string>
#include <vector>

namespace MCA {

struct Configurations {
    std::string calibrationFilePath;
    std::string inputPath;
    std::string outputPath;
    int startFrame, endFrame;
};

struct SequenceInfo {
    int width, height;
    float diameter;
    float rotationAngle;
    int rows, cols;
    std::vector<float[]> centers;
};

} // namespace MCA