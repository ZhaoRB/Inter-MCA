#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <opencv2/core.hpp>

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <config_file_path>" << std::endl;
    return 1;
  }

  const char *configFilePath = argv[1];
  std::ifstream configFile(configFilePath);
  if (!configFile) {
    std::cerr << "Error: Cannot open file " << configFilePath << std::endl;
    return 1;
  }

  configFile.close();
  return 0;
}
