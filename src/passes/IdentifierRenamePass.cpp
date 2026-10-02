#include "../../include/passes/IdentifierRenamePass.h"
#include "../../include/core/Logger.h"

#include <vector>
#include <string>
#include <cctype>

namespace deobf {
    namespace passes {

        using namespace deobf::core;

        static const std::vector<std::pair<std::string, std::string>>& baseMap() {
            static const std::vector<std::pair<std::string, std::string>> m = {
                {"залупа",   "fn_add"},
                {"пиписька", "fn_sub"},
                {"жопа",     "fn_mul"},
                {"сиськи",   "fn_div"},
                {"хуй",      "int64_t"},
                {"дабл",     "double"},
                {"строка",   "std::string"},
                {"буква",    "char"},
                {"КРЧ",      "int"},
                {"БРАТИШ",   "main"},
                {"ЧЁ",       "using"},
                {"ТИПО",     "namespace"},
                {"СТД",      "std"},
                {"ВЫВОД",    "cout"},
                {"ВВОД",     "cin"},
                {"КОНЕЦ",    "endl"},
                {"ЕСЛИ",     "if"},
                {"ИНАЧЕ",    "else"},
                {"ПОКА",     "while"},
                {"ВЕРНИ",    "return"},
                {"НОЛЬ",     "0"},
                {"ДВОЙКА",   "2"},
                {"ПЛЮС",     "+"},
                {"МИНУС",    "-"},
                {"УМНОЖ",    "*"},
                {"ДЕЛИ",     "/"},
                {"MIX",      "mix_hash"},
                {"ADD",      "op_add"},
                {"SUB",      "op_sub"},
                {"MUL",      "op_mul"},
                {"DIV",      "op_div"},
            };
            return m;
        }

        static bool isWordChar(unsigned char c) {
            return std::isalnum(c) || c == '_';
        }

        bool IdentifierRenamePass::apply(std::string& code) {
            bool changed = false;
            int count = 0;

            auto tryReplace = [&](const std::string& from, const std::string& to) {
                if (from.empty()) return;
                std::string before = code;
                size_t pos = 0;
                while ((pos = code.find(from, pos)) != std::string::npos) {
                    bool leftOk = (pos == 0) || !isWordChar((unsigned char)code[pos - 1]);
                    size_t endp = pos + from.size();
                    bool rightOk = (endp >= code.size()) ||
                        !isWordChar((unsigned char)code[endp]);
                    if (leftOk && rightOk) {
                        code.replace(pos, from.size(), to);
                        pos += to.size();
                    }
                    else {
                        pos += from.size();
                    }
                }
                if (code != before) { changed = true; ++count; }
                };

            for (const auto& kv : baseMap()) {
                tryReplace(kv.first, kv.second);
            }
            for (const auto& kv : m_extra) {
                tryReplace(kv.first, kv.second);
            }

            if (!changed) Logger::debug("  [rename] переименований не потребовалось");
            else         Logger::debug("  [rename] всего замен: " + std::to_string(count));

            return changed;
        }

    } // namespace passes
} // namespace deobf