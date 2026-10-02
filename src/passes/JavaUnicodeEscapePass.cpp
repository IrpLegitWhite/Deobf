#include "../../include/passes/JavaUnicodeEscapePass.h"
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
            out.reserve(s.size() + 8);
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

        bool JavaUnicodeEscapePass::apply(std::string& code) {
            bool changed = false;

            // обычная строка, НЕ raw — все backslash удвоены
            std::regex re("\"((?:[^\"\\\\]|\\\\.)*)\"");

            std::string work = code;
            std::string result;
            std::smatch m;
            int safety = 0;

            while (std::regex_search(work, m, re) && safety++ < 2000) {
                result += m.prefix().str();

                std::string raw = m[1].str();
                if (raw.find("\\u") == std::string::npos) {
                    result += m[0].str();
                    work = m.suffix().str();
                    continue;
                }

                std::string decoded;
                bool ok = true;
                size_t i = 0;
                while (i < raw.size()) {
                    if (raw[i] != '\\') {
                        decoded += raw[i++];
                        continue;
                    }
                    if (i + 1 >= raw.size()) {
                        ok = false;
                        break;
                    }
                    char n = raw[i + 1];

                    if (n == 'u') {
                        if (i + 5 >= raw.size()) {
                            ok = false;
                            break;
                        }
                        char hex[5] = { raw[i + 2], raw[i + 3], raw[i + 4], raw[i + 5], 0 };
                        try {
                            uint32_t cp = (uint32_t)std::stoul(hex, nullptr, 16);
                            appendUtf8(decoded, cp);
                            i += 6;
                            continue;
                        }
                        catch (...) {
                            ok = false;
                            break;
                        }
                    }
                    else {
                        decoded += raw[i++];
                        if (i < raw.size()) decoded += raw[i++];
                    }
                }

                if (ok && !decoded.empty()) {
                    result += "\"";
                    result += escapeJavaLocal(decoded);
                    result += "\"";
                    Logger::debug("  [java-unicode] decoded");
                    changed = true;
                }
                else {
                    result += m[0].str();
                }
                work = m.suffix().str();
            }
            result += work;
            code = result;

            if (!changed) {
                Logger::debug("  [java-unicode] not found");
            }
            return changed;
        }

    } // namespace passes
} // namespace deobf