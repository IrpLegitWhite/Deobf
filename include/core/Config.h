#pragma once
#include <string>
#include <vector>
#include <map>

namespace deobf {
    namespace core {

        struct Config {
            std::string inputPath;
            std::string outputPath;
            std::string language = "auto";
            bool verbose = false;
            bool dryRun = false;
            std::vector<std::string> enabledPasses;
            std::vector<std::string> disabledPasses;
            std::map<std::string, std::string> renameMap;

            static Config parse(int argc, char** argv);
            void applyRulesFile(const std::string& path);
        };

    } // namespace core
} // namespace deobf