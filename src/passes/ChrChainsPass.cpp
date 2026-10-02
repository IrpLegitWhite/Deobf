#include "../../include/passes/ChrChainsPass.h"
#include "../../include/core/Logger.h"
#include "../../include/core/Utils.h"

#include <regex>
#include <string>
#include <vector>

namespace deobf {
    namespace passes {

        using namespace deobf::core;

        static bool replaceChrAddChain(std::string& code) {
            std::regex re(R"(chr\s*\(\s*(\d+)\s*\)(?:\s*\+\s*chr\s*\(\s*(\d+)\s*\))+)");

            bool changed = false;
            std::string work = code;
            std::string result;
            std::smatch m;
            int safety = 0;

            while (std::regex_search(work, m, re) && safety++ < 1000) {
                result += m.prefix().str();

                std::string expr = m[0].str();
                std::string text;
                std::regex numRe(R"(chr\s*\(\s*(\d+)\s*\))");
                auto b = std::sregex_iterator(expr.begin(), expr.end(), numRe);
                auto e = std::sregex_iterator();
                for (auto it = b; it != e; ++it) {
                    int v = std::stoi((*it)[1]);
                    if (v >= 0 && v <= 255) text += static_cast<char>(v);
                }

                result += "\"";
                result += utils::escapeCpp(text);
                result += "\"";
                Logger::debug("  [chr-chains] chr() цепочка -> \"" + text + "\"");
                work = m.suffix().str();
                changed = true;
            }
            result += work;
            code = result;
            return changed;
        }

        static bool replaceJoinChrList(std::string& code) {
            bool changed = false;

            {
                std::regex re(
                    R"(""\s*\.\s*join\s*\(\s*\[\s*chr\s*\(\s*\w+\s*\)\s+for\s+\w+\s+in\s+\[([0-9,\s]+)\]\s*\]\s*\))");
                std::string work = code, result;
                std::smatch m;
                int safety = 0;
                while (std::regex_search(work, m, re) && safety++ < 500) {
                    result += m.prefix().str();
                    std::string nums = m[1].str();
                    std::string text;
                    std::regex numRe(R"(\d+)");
                    auto b = std::sregex_iterator(nums.begin(), nums.end(), numRe);
                    auto e = std::sregex_iterator();
                    for (auto it = b; it != e; ++it) {
                        int v = std::stoi((*it)[0]);
                        if (v >= 0 && v <= 255) text += static_cast<char>(v);
                    }
                    result += "\"" + utils::escapeCpp(text) + "\"";
                    Logger::debug("  [chr-chains] join([chr(c) for c in [...]]) -> \"" + text + "\"");
                    work = m.suffix().str();
                    changed = true;
                }
                result += work;
                code = result;
            }

            {
                std::regex re(
                    R"(""\s*\.\s*join\s*\(\s*chr\s*\(\s*\w+\s*\)\s+for\s+\w+\s+in\s+\[([0-9,\s]+)\]\s*\))");
                std::string work = code, result;
                std::smatch m;
                int safety = 0;
                while (std::regex_search(work, m, re) && safety++ < 500) {
                    result += m.prefix().str();
                    std::string nums = m[1].str();
                    std::string text;
                    std::regex numRe(R"(\d+)");
                    auto b = std::sregex_iterator(nums.begin(), nums.end(), numRe);
                    auto e = std::sregex_iterator();
                    for (auto it = b; it != e; ++it) {
                        int v = std::stoi((*it)[0]);
                        if (v >= 0 && v <= 255) text += static_cast<char>(v);
                    }
                    result += "\"" + utils::escapeCpp(text) + "\"";
                    Logger::debug("  [chr-chains] join(chr(c) for c in [...]) -> \"" + text + "\"");
                    work = m.suffix().str();
                    changed = true;
                }
                result += work;
                code = result;
            }

            {
                std::regex re(
                    R"(""\s*\.\s*join\s*\(\s*map\s*\(\s*chr\s*,\s*\[([0-9,\s]+)\]\s*\))");
                std::string work = code, result;
                std::smatch m;
                int safety = 0;
                while (std::regex_search(work, m, re) && safety++ < 500) {
                    result += m.prefix().str();
                    std::string nums = m[1].str();
                    std::string text;
                    std::regex numRe(R"(\d+)");
                    auto b = std::sregex_iterator(nums.begin(), nums.end(), numRe);
                    auto e = std::sregex_iterator();
                    for (auto it = b; it != e; ++it) {
                        int v = std::stoi((*it)[0]);
                        if (v >= 0 && v <= 255) text += static_cast<char>(v);
                    }
                    result += "\"" + utils::escapeCpp(text) + "\"";
                    Logger::debug("  [chr-chains] join(map(chr, [...])) -> \"" + text + "\"");
                    work = m.suffix().str();
                    changed = true;
                }
                result += work;
                code = result;
            }

            return changed;
        }

        bool ChrChainsPass::apply(std::string& code) {
            bool c1 = replaceChrAddChain(code);
            bool c2 = replaceJoinChrList(code);
            if (!c1 && !c2) Logger::debug("  [chr-chains] chr-цепочек не найдено");
            return c1 || c2;
        }

    } // namespace passes
} // namespace deobf