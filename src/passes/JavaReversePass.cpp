#include "../../include/passes/JavaReversePass.h"
#include "../../include/core/Logger.h"
#include <regex>
#include <string>
#include <vector>
#include <cstdio>

namespace deobf {
    namespace passes {

        using namespace deobf::core;

        static std::string decodeJavaEscapesLocal(const std::string& in) {
            std::string out;
            out.reserve(in.size());
            for (size_t i = 0; i < in.size(); ++i) {
                if (in[i] != '\\' || i + 1 >= in.size()) { out += in[i]; continue; }
                char n = in[i + 1];
                switch (n) {
                case 'n': out += '\n'; i++; break;
                case 't': out += '\t'; i++; break;
                case 'r': out += '\r'; i++; break;
                case '0': out += '\0'; i++; break;
                case '\\': out += '\\'; i++; break;
                case '"': out += '"'; i++; break;
                case '\'': out += '\''; i++; break;
                default: out += in[i]; break;
                }
            }
            return out;
        }

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

        static std::string reverseUtf8Local(const std::string& s) {
            std::vector<std::string> cps;
            for (size_t i = 0; i < s.size(); ) {
                unsigned char c = (unsigned char)s[i];
                size_t len = 1;
                if ((c & 0x80) == 0x00) len = 1;
                else if ((c & 0xE0) == 0xC0) len = 2;
                else if ((c & 0xF0) == 0xE0) len = 3;
                else if ((c & 0xF8) == 0xF0) len = 4;
                if (i + len > s.size()) len = 1;
                cps.push_back(s.substr(i, len));
                i += len;
            }
            std::string out;
            for (auto it = cps.rbegin(); it != cps.rend(); ++it) out += *it;
            return out;
        }

        bool JavaReversePass::apply(std::string& code) {
            bool changed = false;

            std::regex re(
                "new\\s+String(?:Builder|Buffer)\\s*\\(\\s*\"((?:[^\"\\\\]|\\\\.)*)\"\\s*\\)\\s*\\.\\s*reverse\\s*\\(\\s*\\)\\s*\\.\\s*toString\\s*\\(\\s*\\)");

            std::string work = code;
            std::string result;
            std::smatch m;
            int safety = 0;

            while (std::regex_search(work, m, re) && safety++ < 500) {
                result += m.prefix().str();
                std::string raw = m[1].str();
                std::string decoded = decodeJavaEscapesLocal(raw);
                std::string reversed = reverseUtf8Local(decoded);
                result += "\"";
                result += escapeJavaLocal(reversed);
                result += "\"";
                Logger::debug("  [java-reverse] OK");
                changed = true;
                work = m.suffix().str();
            }
            result += work;
            code = result;

            if (!changed) Logger::debug("  [java-reverse] not found");
            return changed;
        }

    } // namespace passes
} // namespace deobf