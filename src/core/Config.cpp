#include "../../include/core/Config.h"
#include "../../include/core/File.h"
#include "../../include/core/Logger.h"
#include "../../include/core/Utils.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

namespace deobf {
    namespace core {

        Config Config::parse(int argc, char** argv) {
            Config cfg;
            std::vector<std::string> args(argv + 1, argv + argc);

            for (size_t i = 0; i < args.size(); ++i) {
                const auto& a = args[i];

                if (a == "-h" || a == "--help") {
                    std::cout << "deobf — деобусификатор\n";
                    std::exit(0);
                }
                else if (a == "--lang" && i + 1 < args.size()) { cfg.language = args[++i]; }
                else if (a == "--verbose" || a == "-v") { cfg.verbose = true; }
                else if (a == "--dry-run") { cfg.dryRun = true; }
                else if (a == "--enable" && i + 1 < args.size()) {
                    std::stringstream ss(args[++i]);
                    std::string tok;
                    while (std::getline(ss, tok, ',')) {
                        if (!tok.empty()) cfg.enabledPasses.push_back(tok);
                    }
                }
                else if (a == "--disable" && i + 1 < args.size()) {
                    std::stringstream ss(args[++i]);
                    std::string tok;
                    while (std::getline(ss, tok, ',')) {
                        if (!tok.empty()) cfg.disabledPasses.push_back(tok);
                    }
                }
                else if (a == "--rules" && i + 1 < args.size()) {
                    cfg.applyRulesFile(args[++i]);
                }
                else if (!a.empty() && a[0] != '-') {
                    if (cfg.inputPath.empty())       cfg.inputPath = a;
                    else if (cfg.outputPath.empty()) cfg.outputPath = a;
                }
            }

            if (cfg.outputPath.empty()) cfg.outputPath = cfg.inputPath + ".clean";
            if (cfg.verbose) Logger::setLevel(LogLevel::Debug);

            return cfg;
        }

        void Config::applyRulesFile(const std::string& path) {
            std::ifstream f(path);
            if (!f) {
                Logger::warn("Не могу открыть rules-файл: " + path);
                return;
            }
            std::string line;
            int count = 0;
            while (std::getline(f, line)) {
                line = utils::trim(line);
                if (line.empty() || line[0] == '#') continue;

                std::istringstream is(line);
                std::string cmd, from, arrow, to;
                is >> cmd >> from >> arrow >> to;

                if (cmd == "rename" && arrow == "->" && !from.empty() && !to.empty()) {
                    renameMap[from] = to;
                    ++count;
                }
            }
            Logger::info("Загружено правил из " + path + ": " + std::to_string(count));
        }

    } // namespace core
} // namespace deobf