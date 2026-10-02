#include "../../include/passes/EmptyLoopPass.h"
#include "../../include/core/Logger.h"

#include <regex>
#include <string>

namespace deobf {
    namespace passes {

        using namespace deobf::core;

        bool EmptyLoopPass::apply(std::string& code) {
            bool changed = false;

            {
                std::regex re(
                    R"(for\s*\(\s*(?:int|size_t|unsigned|long)\s+\w+\s*=\s*0\s*;\s*\w+\s*<\s*1\s*;\s*\+\+\w+\s*\)\s*[^;{}]*;)");
                std::string before = code;
                code = std::regex_replace(code, re, "/* [empty-loops] удалён пустой цикл */");
                if (code != before) changed = true;
            }
            {
                std::regex re(
                    R"(for\s*\(\s*(?:int|size_t|unsigned|long)\s+\w+\s*=\s*0\s*;\s*\w+\s*<\s*1\s*;\s*\w+\+\+\s*\)\s*[^;{}]*;)");
                std::string before = code;
                code = std::regex_replace(code, re, "/* [empty-loops] удалён пустой цикл */");
                if (code != before) changed = true;
            }
            {
                std::regex re(R"(while\s*\(\s*(?:false|0)\s*\)\s*\{[^{}]*\})");
                std::string before = code;
                code = std::regex_replace(code, re, "/* [empty-loops] удалён мёртвый while */");
                if (code != before) changed = true;
            }

            if (changed) Logger::debug("  [empty-loops] удалены пустые/мёртвые циклы");
            else         Logger::debug("  [empty-loops] пустых циклов не найдено");

            return changed;
        }

    } // namespace passes
} // namespace deobf