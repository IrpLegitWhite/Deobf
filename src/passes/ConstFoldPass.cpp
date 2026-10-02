#include "../../include/passes/ConstFoldPass.h"
#include "../../include/core/Logger.h"

#include <regex>
#include <string>

namespace deobf {
    namespace passes {

        using namespace deobf::core;

        bool ConstFoldPass::apply(std::string& code) {
            bool changed = false;

            // сложение
            {
                std::regex re(R"(\b(\d+)\s*\+\s*(\d+)\b)");
                std::string work = code, result;
                std::smatch m;
                int safety = 0;
                while (std::regex_search(work, m, re) && safety++ < 5000) {
                    result += m.prefix().str();
                    try {
                        long long a = std::stoll(m[1]), b = std::stoll(m[2]);
                        result += std::to_string(a + b);
                        changed = true;
                    }
                    catch (...) { result += m[0].str(); }
                    work = m.suffix().str();
                }
                result += work;
                code = result;
            }

            // умножение
            {
                std::regex re(R"(\b(\d+)\s*\*\s*(\d+)\b)");
                std::string work = code, result;
                std::smatch m;
                int safety = 0;
                while (std::regex_search(work, m, re) && safety++ < 5000) {
                    result += m.prefix().str();
                    try {
                        long long a = std::stoll(m[1]), b = std::stoll(m[2]);
                        result += std::to_string(a * b);
                        changed = true;
                    }
                    catch (...) { result += m[0].str(); }
                    work = m.suffix().str();
                }
                result += work;
                code = result;
            }

            // вычитание
            {
                std::regex re(R"(\b(\d+)\s*-\s*(\d+)\b)");
                std::string work = code, result;
                std::smatch m;
                int safety = 0;
                while (std::regex_search(work, m, re) && safety++ < 5000) {
                    result += m.prefix().str();
                    try {
                        long long a = std::stoll(m[1]), b = std::stoll(m[2]);
                        result += std::to_string(a - b);
                        changed = true;
                    }
                    catch (...) { result += m[0].str(); }
                    work = m.suffix().str();
                }
                result += work;
                code = result;
            }

            if (changed) Logger::debug("  [const-fold] константы свёрнуты");
            else         Logger::debug("  [const-fold] нечего сворачивать");
            return changed;
        }

    } // namespace passes
} // namespace deobf