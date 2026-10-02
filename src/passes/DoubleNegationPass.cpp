#include "../../include/passes/DoubleNegationPass.h"
#include "../../include/core/Logger.h"

#include <regex>
#include <string>

namespace deobf {
    namespace passes {

        using namespace deobf::core;

        bool DoubleNegationPass::apply(std::string& code) {
            bool changed = false;

            {
                std::regex re(R"(-\s*\(\s*-\s*\(\s*(\w+)\s*\)\s*\))");
                std::string b = code;
                code = std::regex_replace(code, re, "$1");
                if (code != b) { changed = true; Logger::debug("  [double-negation] -(-(x)) -> x"); }
            }
            {
                std::regex re(R"(-\s*\(\s*-\s*(\w+)\s*\))");
                std::string b = code;
                code = std::regex_replace(code, re, "$1");
                if (code != b) { changed = true; Logger::debug("  [double-negation] -(-x) -> x"); }
            }
            {
                std::regex re(R"(-\s*-\s*(\w+))");
                std::string b = code;
                code = std::regex_replace(code, re, "$1");
                if (code != b) { changed = true; Logger::debug("  [double-negation] --x -> x"); }
            }
            {
                std::regex re(R"(!\s*\(\s*!\s*\(\s*(\w+)\s*\)\s*\))");
                std::string b = code;
                code = std::regex_replace(code, re, "$1");
                if (code != b) { changed = true; Logger::debug("  [double-negation] !(!(x)) -> x"); }
            }
            {
                std::regex re(R"(!!(\w+))");
                std::string b = code;
                code = std::regex_replace(code, re, "$1");
                if (code != b) { changed = true; Logger::debug("  [double-negation] !!x -> x"); }
            }

            if (!changed) Logger::debug("  [double-negation] двойных отрицаний не найдено");
            return changed;
        }

    } // namespace passes
} // namespace deobf