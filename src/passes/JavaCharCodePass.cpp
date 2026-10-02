#include "../../include/passes/JavaCharCodePass.h"
#include "../../include/core/Logger.h"
#include <regex>
#include <string>
#include <cstdint>
#include <cstdio>

namespace deobf {
    namespace passes {

        using namespace deobf::core;

        static std::string escapeJavaLocal(const std::string& s) {
            std::string out;
            for (unsigned char c : s) {
                switch (c) {
                case '\\': out += "\\\\"; break;
                case '"':  out += "\\\""; break;
                case '\n': out += "\\n";  break;
                case '\r': out += "\\r";  break;
                case '\t': out += "\\t";  break;
                default:
                    if (c < 0x20 || c == 0x7F) {
                        char buf[8];
                        std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                        out += buf;
                    }
                    else out += (char)c;
                }
            }
            return out;
        }

        static void appendUtf8(std::string& out, uint32_t cp) {
            if (cp <= 0x7F) {
                out.push_back((char)cp);
            }
            else if (cp <= 0x7FF) {
                out.push_back((char)(0xC0 | (cp >> 6)));
                out.push_back((char)(0x80 | (cp & 0x3F)));
            }
            else if (cp <= 0xFFFF) {
                out.push_back((char)(0xE0 | (cp >> 12)));
                out.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
                out.push_back((char)(0x80 | (cp & 0x3F)));
            }
            else if (cp <= 0x10FFFF) {
                out.push_back((char)(0xF0 | (cp >> 18)));
                out.push_back((char)(0x80 | ((cp >> 12) & 0x3F)));
                out.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
                out.push_back((char)(0x80 | (cp & 0x3F)));
            }
        }

        static bool parseNumLocal(const std::string& s, uint32_t& out) {
            try {
                size_t pos = 0;
                unsigned long v;
                if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
                    v = std::stoul(s, &pos, 16);
                else
                    v = std::stoul(s, &pos, 10);
                if (pos != s.size()) return false;
                out = (uint32_t)v;
                return true;
            }
            catch (...) { return false; }
        }

        bool JavaCharCodePass::apply(std::string& code) {
            bool changed = false;

            std::regex re(
                "new\\s+String\\s*\\(\\s*new\\s+(char|byte)\\s*\\[\\s*\\]\\s*\\{\\s*([0-9xXa-fA-F,\\s\\-]+)\\s*\\}\\s*\\)");

            std::regex numRe("-?(?:0[xX][0-9a-fA-F]+|\\d+)");

            std::string work = code;
            std::string result;
            std::smatch m;
            int safety = 0;

            while (std::regex_search(work, m, re) && safety++ < 500) {
                result += m.prefix().str();

                std::string type = m[1].str();
                std::string text;
                bool ok = true;
                int count = 0;

                auto b = std::sregex_iterator(m[2].first, m[2].second, numRe);
                auto e = std::sregex_iterator();
                for (auto it = b; it != e; ++it) {
                    uint32_t v;
                    if (!parseNumLocal((*it)[0], v)) { ok = false; break; }
                    if (v > 0x10FFFF) { ok = false; break; }
                    if (type == "byte" && v > 0xFF) { ok = false; break; }
                    appendUtf8(text, v);
                    ++count;
                }

                if (ok && count > 0 && !text.empty()) {
                    result += "\"";
                    result += escapeJavaLocal(text);
                    result += "\"";
                    Logger::debug("  [java-charcode] " + type + "[" + std::to_string(count) + "]");
                    changed = true;
                }
                else {
                    result += m[0].str();
                }
                work = m.suffix().str();
            }
            result += work;
            code = result;

            if (!changed) Logger::debug("  [java-charcode] not found");
            return changed;
        }

    } // namespace passes
} // namespace deobf