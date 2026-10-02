#include "../../include/passes/JsDeadFuncPass.h"
#include "../../include/core/Logger.h"
#include <regex>
#include <string>
#include <vector>
#include <algorithm>

namespace deobf {
    namespace passes {
        using namespace deobf::core;

        struct Range { size_t start; size_t end; std::string name; };

        static size_t findBraceEnd(const std::string& code, size_t braceStart) {
            int depth = 0;
            for (size_t i = braceStart; i < code.size(); ++i) {
                if (code[i] == '{') ++depth;
                else if (code[i] == '}') { --depth; if (depth == 0) return i + 1; }
            }
            return std::string::npos;
        }

        bool JsDeadFuncPass::apply(std::string& code) {
            std::regex fnRe(R"(function\s+(\w*(?:_dead|_trap|dead_|trap_|_dummy|dummy_)\w*)\s*\([^)]*\)\s*\{)");
            std::vector<Range> toRemove;
            auto begin = std::sregex_iterator(code.begin(), code.end(), fnRe);
            auto end = std::sregex_iterator();
            for (auto it = begin; it != end; ++it) {
                std::smatch m = *it;
                size_t start = m.position(0);
                size_t braceStart = code.find('{', start);
                if (braceStart == std::string::npos) continue;
                size_t braceEnd = findBraceEnd(code, braceStart);
                if (braceEnd == std::string::npos) continue;
                if (braceEnd < code.size() && code[braceEnd] == '\n') ++braceEnd;
                toRemove.push_back({ start, braceEnd, m[1] });
            }
            if (toRemove.empty()) { Logger::debug("  [js-funcs] не найдено"); return false; }
            std::sort(toRemove.begin(), toRemove.end(), [](const Range& a, const Range& b) { return a.start > b.start; });
            for (const auto& r : toRemove) { code.erase(r.start, r.end - r.start); Logger::debug("  [js-funcs] удалена: " + r.name); }
            return true;
        }
    }
}