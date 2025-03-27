#include "ConfigParser.h"
#include <fstream>

Config ConfigParser::Parse(const std::string filename) {
    std::ifstream confFile(filename);
    if (!confFile.is_open()) {
        throw std::runtime_error("Error opening configuration file '" + filename + "'");
    }

    Config config{};

    std::string line;
    while (std::getline(confFile, line)) {
        size_t commentPos = line.find("#");
        if (commentPos == 0) {
            continue;
        }

        size_t delimiterPos = line.find('=');
        if (delimiterPos == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, delimiterPos);
        std::string value = line.substr(delimiterPos + 1);

        // Remove leading/trailing whitespace
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t\r") + 1);

        // Process the extracted key and value
        if (key == "apiServerUri") {
            config.apiServerUrl = value;
        }
        else if (key == "apiServerPassword") {
            config.apiServerPassword = value;
        }
    }

    confFile.close();

    return config;
}