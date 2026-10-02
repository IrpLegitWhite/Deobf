#include "../../include/passes/CharMathPass.h"
#include "../../include/core/Logger.h"
#include "../../include/core/Utils.h"

#include <regex>
#include <string>
#include <vector>
#include <cctype>

namespace deobf {
    namespace passes {

        using namespace deobf::core;

        bool CharMathPass::apply(std::string& code) {
            std::regex re(R"((?:char\s*\(\s*(\d+)\s*\)\s*\+\s*){2,}char\s*\(\s*(\d+)\s*\))");

            bool changed = false;
            std::string work = code;
            std::string result;
            std::smatch m;
            int safety = 0;

            while (std::regex_search(work, m, re) && safety++ < 1000) {
                result += m.prefix().str();

                std::string expr = m[0].str();
                std::string text;
                std::regex numRe(R"(char\s*\(\s*(\d+)\s*\))");
                auto b = std::sregex_iterator(expr.begin(), expr.end(), numRe);
                auto e = std::sregex_iterator();
                for (auto it = b; it != e; ++it) {
                    int v = std::stoi((*it)[1]);
                    if (v >= 0x20 && v <= 0x7E) text += static_cast<char>(v);
                    else text += '?';
                }

                result += "\"" + utils::escapeCpp(text) + "\"";
                Logger::debug("  [char-math] char() цепочка -> \"" + text + "\"");
                work = m.suffix().str();
                changed = true;
            }
            result += work;
            code = result;

            if (!changed) Logger::debug("  [char-math] char() цепочек не найдено");
            return changed;
        }

    } // namespace passes
} // namespace deobf