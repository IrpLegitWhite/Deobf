#include "../../include/passes/FakeBranchPass.h"
#include "../../include/core/Logger.h"

#include <regex>
#include <string>

namespace deobf {
    namespace passes {

        using namespace deobf::core;

        bool FakeBranchPass::apply(std::string& code) {
            bool changed = false;

            {
                std::regex re(R"(if\s*\(\s*(?:1|true)\s*\)\s*\{([^{}]*)\})");
                std::string b = code;
                code = std::regex_replace(code, re, "{$1}");
                if (code != b) { changed = true; Logger::debug("  [fake-branches] if (1) -> тело"); }
            }
            {
                std::regex re(R"(if\s*\(\s*(?:0|false)\s*\)\s*\{[^{}]*\})");
                std::string b = code;
                code = std::regex_replace(code, re, "/* [fake-branches] if(0) удалено */");
                if (code != b) { changed = true; Logger::debug("  [fake-branches] if (0) -> удалено"); }
            }
            {
                std::regex re(R"(while\s*\(\s*(?:0|false)\s*\)\s*\{[^{}]*\})");
                std::string b = code;
                code = std::regex_replace(code, re, "/* [fake-branches] while(0) удалено */");
                if (code != b) { changed = true; Logger::debug("  [fake-branches] while (0) -> удалено"); }
            }
            {
                std::regex re(R"(while\s*\(\s*(?:1|true)\s*\)\s*\{([^{}]*)\})");
                std::string b = code;
                code = std::regex_replace(code, re, "{$1}");
                if (code != b) { changed = true; Logger::debug("  [fake-branches] while (1) -> тело"); }
            }

            if (!changed) Logger::debug("  [fake-branches] мёртвых ветвлений не найдено");
            return changed;
        }

    } // namespace passes
} // namespace deobf