#include "../../include/passes/JavaMbaPass.h"
#include "../../include/core/Logger.h"
#include <regex>
#include <string>
#include <vector>

namespace deobf {
    namespace passes {

        using namespace deobf::core;

        bool JavaMbaPass::apply(std::string& code) {
            bool changed = false;

            struct Rule {
                std::regex re;
                std::string repl;
            };

            // ★ УБРАНЫ опасные правила: x + 0, x - 0 (могут ломать строки).
            //   ОСТАВЛЕНЫ только безопасные: * 1, / 1, | 0, ^ 0, & -1.
            std::vector<Rule> rules = {
                // (x * 1) -> x
                { std::regex("\\(\\s*(\\w+)\\s*\\*\\s*1\\s*\\)"),           "$1" },
                // (x) * 1 -> x
                { std::regex("\\(\\s*(\\w+)\\s*\\)\\s*\\*\\s*1(?!\\d)"),    "$1" },
                // (x / 1) -> x
                { std::regex("\\(\\s*(\\w+)\\s*/\\s*1\\s*\\)"),             "$1" },
                // (x) / 1 -> x
                { std::regex("\\(\\s*(\\w+)\\s*\\)\\s*/\\s*1(?!\\d)"),      "$1" },
                // (x | 0) -> x
                { std::regex("\\(\\s*(\\w+)\\s*\\|\\s*0\\s*\\)"),           "$1" },
                // (x ^ 0) -> x
                { std::regex("\\(\\s*(\\w+)\\s*\\^\\s*0\\s*\\)"),           "$1" },
                // (x & -1) -> x
                { std::regex("\\(\\s*(\\w+)\\s*\\&\\s*-1\\s*\\)"),          "$1" },
                // x * 1 (без скобок)
                { std::regex("\\b(\\w+)\\s*\\*\\s*1(?!\\d)"),               "$1" },
                // x / 1 (без скобок)
                { std::regex("\\b(\\w+)\\s*/\\s*1(?!\\d)"),                 "$1" },
            };

            for (auto& rule : rules) {
                std::string work = code;
                std::string result;
                std::smatch m;
                int safety = 0;
                bool any = false;
                while (std::regex_search(work, m, rule.re) && safety++ < 1000) {
                    result += m.prefix().str();
                    result += m[1].str();
                    work = m.suffix().str();
                    any = true;
                }
                result += work;
                if (any) {
                    code = result;
                    changed = true;
                }
            }

            if (!changed) Logger::debug("  [java-mba] not found");
            return changed;
        }

    } // namespace passes
} // namespace deobf