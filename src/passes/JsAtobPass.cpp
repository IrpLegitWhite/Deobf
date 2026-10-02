#include "../../include/passes/JsAtobPass.h"
#include "../../include/core/Logger.h"
#include "../../include/core/Utils.h"
#include <regex>
#include <string>
#include <cstdint>

namespace deobf {
    namespace passes {
        using namespace deobf::core;

        static const char kB64[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        // Корректный base64-декодер с валидацией padding
        static bool tryBase64(const std::string& in, std::string& out) {
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
                        // padding допустим только в последних двух позициях
                        if (j < 2) return false;
                        // и только в самом конце строки
                        if (i + 4 != in.size()) return false;
                        // и все оставшиеся символы — тоже '='
                        for (int k = j + 1; k < 4; ++k)
                            if (in[i + k] != '=') return false;
                        break;
                    }
                    const char* p = std::strchr(kB64, c);
                    if (!p) return false;
                    v[j] = static_cast<int>(p - kB64);
                }

                if (pad > 2) return false;

                out.push_back(static_cast<char>((v[0] << 2) | (v[1] >> 4)));
                if (pad < 2) out.push_back(static_cast<char>(((v[1] & 0x0F) << 4) | (v[2] >> 2)));
                if (pad < 1) out.push_back(static_cast<char>(((v[2] & 0x03) << 6) | v[3]));
            }
            return !out.empty();
        }

        // Экранирование для вставки в JS-строку (двойные кавычки)
        static std::string escapeJs(const std::string& s) {
            std::string out;
            out.reserve(s.size() + 8);
            for (unsigned char c : s) {
                switch (c) {
                case '\\': out += "\\\\"; break;
                case '"':  out += "\\\""; break;
                case '\n': out += "\\n";  break;
                case '\r': out += "\\r";  break;
                case '\t': out += "\\t";  break;
                case '\b': out += "\\b";  break;
                case '\f': out += "\\f";  break;
                default:
                    if (c < 0x20 || c == 0x7F) {
                        char buf[8];
                        std::snprintf(buf, sizeof(buf), "\\x%02x", c);
                        out += buf;
                    }
                    else {
                        out += static_cast<char>(c);
                    }
                }
            }
            return out;
        }

        // Проверка, что байты — валидный UTF-8 (хотя бы грубо)
        static bool isValidUtf8(const std::string& s) {
            size_t i = 0;
            while (i < s.size()) {
                unsigned char c = static_cast<unsigned char>(s[i]);
                size_t len;
                if ((c & 0x80) == 0x00) len = 1;
                else if ((c & 0xE0) == 0xC0) len = 2;
                else if ((c & 0xF0) == 0xE0) len = 3;
                else if ((c & 0xF8) == 0xF0) len = 4;
                else return false;

                if (i + len > s.size()) return false;
                for (size_t k = 1; k < len; ++k) {
                    unsigned char cc = static_cast<unsigned char>(s[i + k]);
                    if ((cc & 0xC0) != 0x80) return false;
                }
                i += len;
            }
            return true;
        }

        // Проверка «похоже на текст» (для atob в JS — latin1)
        static bool looksLikeText(const std::string& s) {
            if (s.empty()) return false;
            int good = 0, bad = 0;
            for (unsigned char c : s) {
                if (c == 0) { bad++; continue; }
                if (c == ' ' || c == '\n' || c == '\t' || c == '\r') { good++; continue; }
                if (c >= 0x20 && c < 0x7F) { good++; continue; }
                // latin1 printable
                if (c >= 0xA0) { good++; continue; }
                bad++;
            }
            int total = good + bad;
            if (total == 0) return false;
            return good * 10 >= total * 7;
        }

        bool JsAtobPass::apply(std::string& code) {
            bool changed = false;

            // atob("...") — поддерживаем ", ', ` и пробелы
            std::regex re(R"(\batob\s*\(\s*(["'`])([A-Za-z0-9+/=\s]+)\1\s*\))");

            std::string work = code, result;
            std::smatch m;
            int safety = 0;

            while (std::regex_search(work, m, re) && safety++ < 500) {
                result += m.prefix().str();

                std::string payload = m[2].str();
                // убираем whitespace внутри base64 (переносы строк)
                payload.erase(std::remove_if(payload.begin(), payload.end(),
                    [](unsigned char c) { return std::isspace(c); }),
                    payload.end());

                std::string dec;
                if (tryBase64(payload, dec) && looksLikeText(dec)) {
                    result += "\"" + escapeJs(dec) + "\"";
                    Logger::debug("  [js-atob] -> \"" + dec + "\"");
                    changed = true;
                }
                else {
                    result += m[0].str();
                }
                work = m.suffix().str();
            }
            result += work;
            code = result;

            if (!changed) Logger::debug("  [js-atob] atob не найдено");
            return changed;
        }

    }
} // namespace