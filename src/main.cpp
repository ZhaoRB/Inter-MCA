#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <opencv2/core.hpp>
#include <string>

using json = nlohmann::json;

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <config_file_path>"
              << " <config_name>" << std::endl;
    return 1;
  }

  const char *configFilePath = argv[1];
  std::ifstream configFile(configFilePath);

  if (!configFile) {
    std::cerr << "Error: Cannot open file " << configFilePath << std::endl;
    return 1;
  }

  const std::string configName = argv[2];
  json data;
  try {
    configFile >> data;
  } catch (const json::parse_error &e) {
    std::cerr << "Error: Failed to parse JSON file. " << e.what() << std::endl;
    return 1;
  }

  if (data.find(configName) == data.end()) {
    std::cerr << "Error: Config name '" << configName
              << "' not found in the configuration file." << std::endl;
    // std::cerr << "Available config names: ";
    // for (auto it = data.begin(); it != data.end(); ++it) {
    //   std::cerr << it.key() << " ";
    // }
    // std::cerr << std::endl;
    return 1;
  }

  json config = data[configName];

  std::vector<std::string> sequences;
  std::vector<std::string> tasks;
  std::string dataFormat;

  try {
    sequences = config.at("sequences").get<std::vector<std::string>>();
    tasks = config.at("tasks").get<std::vector<std::string>>();
    dataFormat = config.at("dataFormat").get<std::string>();
  } catch (const json::out_of_range &e) {
    std::cerr << "Error: Missing expected key in the config: " << e.what()
              << std::endl;
    return 1;
  } catch (const json::type_error &e) {
    std::cerr << "Error: Type mismatch when accessing config values: "
              << e.what() << std::endl;
    return 1;
  }

  return 0;
}
