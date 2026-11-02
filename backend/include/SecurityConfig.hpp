#pragma once

#include <string>
#include <fstream>
#include <cstdlib>
#include <iostream>

namespace aibuddy {

class SecurityConfig {
public:
    static std::string getGeminiApiKey() {
        // Priority 1: System environment variable
        const char* envVal = std::getenv("GEMINI_API_KEY");
        if (envVal && std::string(envVal).length() > 10) {
            return std::string(envVal);
        }

        // Priority 2: Local .env file in root
        std::string fileKey = readKeyFromEnvFile(".env");
        if (!fileKey.empty()) return fileKey;

        // Priority 3: Local .env file in backend/
        fileKey = readKeyFromEnvFile("backend/.env");
        if (!fileKey.empty()) return fileKey;

        return "";
    }

    static std::string getMaskedKey(const std::string& key) {
        if (key.length() <= 8) return "********";
        return key.substr(0, 6) + "..." + key.substr(key.length() - 4);
    }

private:
    static std::string readKeyFromEnvFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return "";

        std::string line;
        while (std::getline(file, line)) {
            // Trim leading whitespace
            size_t start = line.find_first_not_of(" \t\r\n");
            if (start == std::string::npos || line[start] == '#') continue;

            size_t eqPos = line.find('=', start);
            if (eqPos != std::string::npos) {
                std::string varName = line.substr(start, eqPos - start);
                // Trim trailing whitespace from varName
                size_t endVar = varName.find_last_not_of(" \t");
                if (endVar != std::string::npos) varName = varName.substr(0, endVar + 1);

                if (varName == "GEMINI_API_KEY" || varName == "API_KEY") {
                    std::string val = line.substr(eqPos + 1);
                    size_t valStart = val.find_first_not_of(" \t\"'");
                    size_t valEnd = val.find_last_not_of(" \t\r\n\"'");
                    if (valStart != std::string::npos && valEnd != std::string::npos && valEnd >= valStart) {
                        return val.substr(valStart, valEnd - valStart + 1);
                    }
                }
            }
        }
        return "";
    }
};

} // namespace aibuddy
