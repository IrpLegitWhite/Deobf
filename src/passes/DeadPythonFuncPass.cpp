#include "../../include/passes/DeadPythonFuncPass.h"
#include "../../include/core/Logger.h"

#include <regex>
#include <string>
#include <vector>
#include <algorithm>

namespace deobf {
    namespace passes {

        using namespace deobf::core;

        struct Range {
            size_t start;
            size_t end;
            std::string name;
        };

        bool DeadPythonFuncPass::apply(std::string& code) {
            // Ищем: def _dead_func(): ...  (Python)
            // Функция начинается с `def NAME(...):` и заканчивается на строке с меньшим отступом
            std::regex defRe(R"((^|\n)([ \t]*)def\s+(\w*(?:_dead|_trap|dead_|trap_)\w*)\s*\([^)]*\)\s*:)",
                std::regex::multiline);

            std::vector<Range> toRemove;

            auto begin = std::sregex_iterator(code.begin(), code.end(), defRe);
            auto end = std::sregex_iterator();

            for (auto it = begin; it != end; ++it) {
                std::smatch m = *it;
                size_t start = m.position(0);
                // пропускаем начальный \n если есть
                if (code[start] == '\n') ++start;

                std::string indent = m[2].str();
                std::string fname = m[3].str();

                // Ищем конец: строку с меньшим отступом
                size_t pos = start;
                // пропускаем первую строку
                while (pos < code.size() && code[pos] != '\n') ++pos;
                if (pos < code.size()) ++pos;

                size_t funcEnd = code.size();
                while (pos < code.size()) {
                    // начало строки
                    size_t lineStart = pos;
                    // считаем отступ
                    size_t p = pos;
                    while (p < code.size() && (code[p] == ' ' || code[p] == '\t')) ++p;

                    std::string lineIndent = code.substr(lineStart, p - lineStart);
                    std::string line = code.substr(p, code.find('\n', p) - p);

                    // пустая строка — пропускаем
                    bool isEmpty = line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos;
                    if (isEmpty) {
                        pos = code.find('\n', p);
                        if (pos == std::string::npos) break;
                        ++pos;
                        continue;
                    }

                    // если отступ меньше — функция закончилась
                    if (lineIndent.size() <= indent.size()) {
                        funcEnd = lineStart;
                        break;
                    }

                    pos = code.find('\n', p);
                    if (pos == std::string::npos) { funcEnd = code.size(); break; }
                    ++pos;
                }

                toRemove.push_back({ start, funcEnd, fname });
            }

            if (toRemove.empty()) {
                Logger::debug("  [dead-python-funcs] мёртвых функций не найдено");
                return false;
            }

            std::sort(toRemove.begin(), toRemove.end(),
                [](const Range& a, const Range& b) { return a.start > b.start; });

            for (const auto& r : toRemove) {
                code.erase(r.start, r.end - r.start);
                Logger::debug("  [dead-python-funcs] удалена: " + r.name);
            }

            return true;
        }

    } // namespace passes
} // namespace deobf