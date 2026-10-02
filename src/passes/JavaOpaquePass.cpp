#include "../../include/passes/JavaOpaquePass.h"
#include "../../include/core/Logger.h"
#include <regex>
#include <string>
#include <vector>

namespace deobf {
    namespace passes {

        using namespace deobf::core;

        bool JavaOpaquePass::apply(std::string& code) {
            bool changed = false;

            struct Rule {
                std::regex re;
                std::string repl;
            };

            // ★ Правильные скобки: внешний if (...) + внутреннее (...) == 0
            //   Заменяем ВСЮ конструкцию "((x ^ x) == 0)" на "(true)".
            //   Тогда остаётся "if (true)" — и java-dead-code его уберёт.

            std::vector<Rule> rules = {
                // if ((x*x) >= 0)  →  (true)
                { std::regex("\\(\\(\\s*(\\w+)\\s*\\*\\s*\\1\\s*\\)\\s*>=\\s*0\\s*\\)"),   "(true)" },
                // if ((x*x) < 0)   →  (false)
                { std::regex("\\(\\(\\s*(\\w+)\\s*\\*\\s*\\1\\s*\\)\\s*<\\s*0\\s*\\)"),    "(false)" },
                // if ((x - x) == 0) →  (true)
                { std::regex("\\(\\(\\s*(\\w+)\\s*-\\s*\\1\\s*\\)\\s*==\\s*0\\s*\\)"),     "(true)" },
                // if ((x ^ x) == 0) →  (true)     ★ ГЛАВНОЕ ПРАВИЛО
                { std::regex("\\(\\(\\s*(\\w+)\\s*\\^\\s*\\1\\s*\\)\\s*==\\s*0\\s*\\)"),   "(true)" },
                // if ((x ^ x) != 0) →  (false)
                { std::regex("\\(\\(\\s*(\\w+)\\s*\\^\\s*\\1\\s*\\)\\s*!=\\s*0\\s*\\)"),   "(false)" },
                // if ((x | 0) == x) →  (true)
                { std::regex("\\(\\(\\s*(\\w+)\\s*\\|\\s*0\\s*\\)\\s*==\\s*\\1\\s*\\)"),   "(true)" },
                // if ((x & x) == x) →  (true)
                { std::regex("\\(\\(\\s*(\\w+)\\s*&\\s*\\1\\s*\\)\\s*==\\s*\\1\\s*\\)"),   "(true)" },
                // if ((x | x) == x) →  (true)
                { std::regex("\\(\\(\\s*(\\w+)\\s*\\|\\s*\\1\\s*\\)\\s*==\\s*\\1\\s*\\)"), "(true)" },

                // БЕЗ внешних скобок if — на случай, если java-opaque вызывается вне if:
                // (x*x >= 0) → (true)
                { std::regex("\\(\\s*(\\w+)\\s*\\*\\s*\\1\\s*>=\\s*0\\s*\\)"),   "(true)" },
                // (x - x == 0) → (true)
                { std::regex("\\(\\s*(\\w+)\\s*-\\s*\\1\\s*==\\s*0\\s*\\)"),     "(true)" },
                // (x ^ x == 0) → (true)
                { std::regex("\\(\\s*(\\w+)\\s*\\^\\s*\\1\\s*==\\s*0\\s*\\)"),   "(true)" },
                // ((x) == x) → (true)
                { std::regex("\\(\\s*(\\w+)\\s*\\)\\s*==\\s*\\1"),               "(true)" },
                // (N == N) → (true)
                { std::regex("\\(\\s*(\\d+)\\s*==\\s*\\1\\s*\\)"),               "(true)" },
                // (N != N) → (false)
                { std::regex("\\(\\s*(\\d+)\\s*!=\\s*\\1\\s*\\)"),               "(false)" },
            };

            for (auto& rule : rules) {
                std::string work = code;
                std::string result;
                std::smatch m;
                int safety = 0;
                bool any = false;
                while (std::regex_search(work, m, rule.re) && safety++ < 500) {
                    result += m.prefix().str();
                    result += rule.repl;
                    work = m.suffix().str();
                    any = true;
                }
                result += work;
                if (any) {
                    code = result;
                    changed = true;
                }
            }

            // ★ Убираем лишние двойные скобки: ((true)) → (true)
            {
                std::regex re("\\(\\(\\s*(true|false)\\s*\\)\\)");
                std::string work = code;
                std::string result;
                std::smatch m;
                int safety = 0;
                bool any = false;
                while (std::regex_search(work, m, re) && safety++ < 500) {
                    result += m.prefix().str();
                    result += "(" + m[1].str() + ")";
                    work = m.suffix().str();
                    any = true;
                }
                result += work;
                if (any) {
                    code = result;
                    changed = true;
                }
            }

            if (!changed) Logger::debug("  [java-opaque] not found");
            return changed;
        }

    } // namespace passes
} // namespace deobf