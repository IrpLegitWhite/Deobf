#include "../../include/passes/JavaBase64Pass.h"
#include "../../include/core/Logger.h"
#include <regex>
#include <string>
#include <cstring>
#include <cstdio>

namespace deobf {
    namespace passes {

        using namespace deobf::core;

        static const char kB64[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        static bool tryBase64Local(const std::string& in, std::string& out) {
            if (in.empty() || in.size() % 4 != 0) return false;
            out.clear();
            out.reserve(in.size() / 4 * 3);

            for (size_t i = 0; i < in.size(); i += 4) {
                int v[4];
                int pad = 0;
                for (int j = 0; j < 4; ++j) {
                    char c = in[i + j];
                    if (c == '=') {
                        v[j] = 0;
                        ++pad;
                        if (j < 2) return false;
                        if (i + 4 != in.size()) return false;
                        for (int k = j + 1; k < 4; ++k)
                            if (in[i + k] != '=') return false;
                        break;
                    }
                    const char* p = strchr(kB64, c);
                    if (!p) return false;
                    v[j] = (int)(p - kB64);
                }
                if (pad > 2) return false;
                out.push_back((char)((v[0] << 2) | (v[1] >> 4)));
                if (pad < 2) out.push_back((char)(((v[1] & 0x0F) << 4) | (v[2] >> 2)));
                if (pad < 1) out.push_back((char)(((v[2] & 0x03) << 6) | v[3]));
            }
            return !out.empty();
        }

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

        // ★ УСИЛЕННАЯ проверка: не пропускать бинарный мусор
        static bool looksLikeTextLocal(const std::string& s) {
            if (s.empty()) return false;

            int good = 0, bad = 0;
            for (unsigned char c : s) {
                if (c == 0) return false;

                // Управляющие символы (кроме \n \r \t) — сразу плохо
                if (c < 0x20 && c != '\n' && c != '\r' && c != '\t') {
                    return false;
                }

                if (c == ' ' || c == '\n' || c == '\t' || c == '\r') { good++; continue; }
                if (c >= 0x20 && c < 0x7F) { good++; continue; }
                // UTF-8: ведущий байт 0xC0-0xF7 или continuation 0x80-0xBF
                if (c >= 0xC0 && c <= 0xF7) { good++; continue; }
                if (c >= 0x80 && c <= 0xBF) { good++; continue; }
                bad++;
            }

            int total = good + bad;
            // ★ Требуем 90% хороших (было 70%)
            return total > 0 && good * 10 >= total * 9;
        }

        bool JavaBase64Pass::apply(std::string& code) {
            bool changed = false;

            std::regex re(
                "new\\s+String\\s*\\(\\s*Base64\\s*\\.\\s*get(?:Url|Mime)?Decoder\\s*\\(\\s*\\)\\s*\\.\\s*decode\\s*\\(\\s*\"([A-Za-z0-9+/=_-]+)\"\\s*\\)\\s*\\)");

            std::string work = code;
            std::string result;
            std::smatch m;
            int safety = 0;

            while (std::regex_search(work, m, re) && safety++ < 500) {
                result += m.prefix().str();

                std::string payload = m[1].str();
                for (auto& c : payload) {
                    if (c == '-') c = '+';
                    if (c == '_') c = '/';
                }

                std::string dec;
                if (tryBase64Local(payload, dec) && looksLikeTextLocal(dec)) {
                    result += "\"";
                    result += escapeJavaLocal(dec);
                    result += "\"";
                    Logger::debug("  [java-base64] decoded");
                    changed = true;
                }
                else {
                    result += m[0].str();
                }
                work = m.suffix().str();
            }
            result += work;
            code = result;

            if (!changed) Logger::debug("  [java-base64] not found");
            return changed;
        }

    } // namespace passes
} // namespace deobf