#include "../../include/passes/DeadFunctionPass.h"
#include "../../include/core/Logger.h"

#include <regex>
#include <vector>
#include <algorithm>
#include <string>

namespace deobf {
    namespace passes {

        using namespace deobf::core;

        struct Range {
            size_t start;
            size_t end;
            std::string name;
        };

        static size_t findBodyEnd(const std::string& code, size_t openBrace) {
            int depth = 0;
            bool inStr = false, inChar = false;
            bool inLineComment = false, inBlockComment = false;

            for (size_t i = openBrace; i < code.size(); ++i) {
                char c = code[i];
                char next = (i + 1 < code.size()) ? code[i + 1] : '\0';

                if (inLineComment) { if (c == '\n') inLineComment = false; continue; }
                if (inBlockComment) {
                    if (c == '*' && next == '/') { inBlockComment = false; ++i; }
                    continue;
                }
                if (inStr) {
                    if (c == '\\') { ++i; continue; }
                    if (c == '"') inStr = false;
                    continue;
                }
                if (inChar) {
                    if (c == '\\') { ++i; continue; }
                    if (c == '\'') inChar = false;
                    continue;
                }
                if (c == '/' && next == '/') { inLineComment = true; ++i; continue; }
                if (c == '/' && next == '*') { inBlockComment = true; ++i; continue; }
                if (c == '"') { inStr = true; continue; }
                if (c == '\'') { inChar = true; continue; }

                if (c == '{') ++depth;
                else if (c == '}') {
                    --depth;
                    if (depth == 0) return i + 1;
                }
            }
            return std::string::npos;
        }

        bool DeadFunctionPass::apply(std::string& code) {
            std::regex fnRe(
                R"((?:static\s+)?(?:void|int|double|float|long|short|char|bool|size_t|uint\w*|int\w*)\s+(\w*(?:dead|trap|dummy|stub|garbage|unused)\w*)\s*\([^)]*\)\s*\{)",
                std::regex::icase);

            std::vector<Range> toRemove;
            auto begin = std::sregex_iterator(code.begin(), code.end(), fnRe);
            auto end = std::sregex_iterator();

            for (auto it = begin; it != end; ++it) {
                std::smatch m = *it;
                size_t start = m.position(0);
                size_t braceStart = code.find('{', start);
                if (braceStart == std::string::npos) continue;

                size_t bodyEnd = findBodyEnd(code, braceStart);
                if (bodyEnd == std::string::npos) continue;
                if (bodyEnd < code.size() && code[bodyEnd] == '\n') ++bodyEnd;

                toRemove.push_back({ start, bodyEnd, m[1] });
            }

            if (toRemove.empty()) {
                Logger::debug("  [dead-functions] мёртвых функций не найдено");
                return false;
            }

            std::sort(toRemove.begin(), toRemove.end(),
                [](const Range& a, const Range& b) { return a.start > b.start; });

            for (const auto& r : toRemove) {
                code.erase(r.start, r.end - r.start);
                Logger::debug("  [dead-functions] удалена: " + r.name);
            }

            return true;
        }

    } // namespace passes
} // namespace deobf