#include "../../include/passes/JavaStringXorPass.h"
#include "../../include/core/Logger.h"
#include <regex>
#include <string>
#include <cstdint>
#include <cstdio>

namespace deobf {
    namespace passes {

        using namespace deobf::core;

        static bool parseNumLocal(const std::string& s, long long& out) {
            try {
                size_t pos = 0;
                long long v;
                if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
                    v = std::stoll(s, &pos, 16);
                else
                    v = std::stoll(s, &pos, 10);
                if (pos != s.size()) return false;
                out = v;
                return true;
            }
            catch (...) { return false; }
        }

        bool JavaStringXorPass::apply(std::string& code) {
            bool changed = false;

            // (byte)(A ^ B) / (char)(A ^ B) -> (byte)N / (char)N
            {
                std::regex re(
                    "\\(\\s*(byte|char)\\s*\\)\\s*\\(\\s*(0[xX][0-9a-fA-F]+|\\d+)\\s*\\^\\s*(0[xX][0-9a-fA-F]+|\\d+)\\s*\\)");

                std::string work = code;
                std::string result;
                std::smatch m;
                int safety = 0;

                while (std::regex_search(work, m, re) && safety++ < 2000) {
                    result += m.prefix().str();
                    long long a, b;
                    if (parseNumLocal(m[2].str(), a) && parseNumLocal(m[3].str(), b)) {
                        long long val = (a ^ b) & 0xFF;
                        char buf[64];
                        std::snprintf(buf, sizeof(buf), "(%s)%lld", m[1].str().c_str(), val);
                        result += buf;
                        Logger::debug("  [java-xor] (byte)(...) -> " + std::string(buf));
                        changed = true;
                    }
                    else {
                        result += m[0].str();
                    }
                    work = m.suffix().str();
                }
                result += work;
                code = result;
            }

            // Второй блок УДАЛЁН: мы больше НЕ сворачиваем new byte[]{...} в строку,
            // т.к. это ломает семантику Java (byte[] != String).

            if (!changed) Logger::debug("  [java-xor] not found");
            return changed;
        }

    } // namespace passes
} // namespace deobf