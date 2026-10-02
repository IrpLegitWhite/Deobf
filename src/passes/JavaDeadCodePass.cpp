#include "../../include/passes/JavaDeadCodePass.h"
#include "../../include/core/Logger.h"
#include <regex>
#include <string>

namespace deobf {
    namespace passes {

        using namespace deobf::core;

        bool JavaDeadCodePass::apply(std::string& code) {
            bool changed = false;

            // === if (true) { ... } -> { ... }   (оставляем фигурные скобки) ===
            {
                std::regex re("if\\s*\\(\\s*true\\s*\\)\\s*(\\{)");
                std::string work = code;
                std::string result;
                std::smatch m;
                int safety = 0;
                while (std::regex_search(work, m, re) && safety++ < 500) {
                    result += m.prefix().str();
                    result += m[1].str();
                    work = m.suffix().str();
                    changed = true;
                }
                result += work;
                code = result;
            }

            // === if (true) <stmt>; -> <stmt>;   (однострочный, без скобок) ===
            {
                std::regex re("if\\s*\\(\\s*true\\s*\\)\\s*(\\S[^;\\n]*;)");
                std::string work = code;
                std::string result;
                std::smatch m;
                int safety = 0;
                while (std::regex_search(work, m, re) && safety++ < 500) {
                    result += m.prefix().str();
                    result += m[1].str();
                    work = m.suffix().str();
                    changed = true;
                }
                result += work;
                code = result;
            }

            // === if (false) { ... } -> удаляем блок целиком ===
            {
                std::regex re("if\\s*\\(\\s*false\\s*\\)\\s*\\{");
                std::string work = code;
                std::string result;
                std::smatch m;
                int safety = 0;
                while (std::regex_search(work, m, re) && safety++ < 500) {
                    result += m.prefix().str();
                    size_t pos = m.position() + m.length();
                    int depth = 1;
                    size_t end = pos;
                    while (end < work.size() && depth > 0) {
                        if (work[end] == '{') depth++;
                        else if (work[end] == '}') depth--;
                        if (depth == 0) break;
                        end++;
                    }
                    if (end < work.size() && depth == 0) {
                        work = work.substr(end + 1);
                        changed = true;
                    }
                    else {
                        result += m[0].str();
                        work = m.suffix().str();
                    }
                }
                result += work;
                code = result;
            }

            if (!changed) Logger::debug("  [java-dead] not found");
            return changed;
        }

    } // namespace passes
} // namespace deobf