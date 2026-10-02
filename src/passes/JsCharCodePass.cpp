#include "../../include/passes/JsCharCodePass.h"
#include "../../include/core/Logger.h"
#include "../../include/core/Utils.h"
#include <regex>
#include <string>
#include <cstdint>
#include <cstdio>

namespace deobf {
    namespace passes {
        using namespace deobf::core;

        // ---- Утилиты ----

        // Экранирование для вставки в JS-строку
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

        // Кодирование codepoint'а в UTF-8 и добавление в out
        static void appendUtf8(std::string& out, uint32_t cp) {
            if (cp <= 0x7F) {
                out.push_back(static_cast<char>(cp));
            }
            else if (cp <= 0x7FF) {
                out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
                out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
            }
            else if (cp <= 0xFFFF) {
                out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
                out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
            }
            else if (cp <= 0x10FFFF) {
                out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
                out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
            }
            // > 0x10FFFF — игнорируем
        }

        // Парсинг одного числового литерала (dec или hex)
        static bool parseNum(const std::string& s, uint32_t& out) {
            try {
                size_t pos = 0;
                unsigned long v;
                if (s.size() > 2 && (s[0] == '0') && (s[1] == 'x' || s[1] == 'X'))
                    v = std::stoul(s, &pos, 16);
                else
                    v = std::stoul(s, &pos, 10);
                if (pos != s.size()) return false;
                out = static_cast<uint32_t>(v);
                return true;
            }
            catch (...) {
                return false;
            }
        }

        // Проверка, что строка — валидный UTF-8
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

        // Проверка, что результат — «текст» (без встроенных нулей и управляющих)
        static bool looksLikeText(const std::string& s) {
            if (s.empty()) return false;
            int good = 0, bad = 0;
            for (unsigned char c : s) {
                if (c == 0) return false;                    // \0 → сразу нет
                if (c == ' ' || c == '\n' || c == '\t' || c == '\r') { good++; continue; }
                if (c >= 0x20 && c < 0x7F) { good++; continue; }
                if (c >= 0x80) { good++; continue; }          // UTF-8 continuation или ведущий
                bad++;
            }
            int total = good + bad;
            if (total == 0) return false;
            return good * 10 >= total * 7;
        }

        // ============================================================
        bool JsCharCodePass::apply(std::string& code) {
            bool changed = false;

            // Проверка: не переопределён ли String.fromCharCode / fromCodePoint в коде
            bool redefined =
                std::regex_search(code,
                    std::regex(R"(String\s*\.\s*from(?:CharCode|CodePoint)\s*=)"));

            if (redefined) {
                Logger::debug("  [js-charcode] String.fromCharCode переопределена — пропуск");
                return false;
            }

            // String.fromCharCode(72, 101, ...) или String.fromCodePoint(0x1F600, ...)
            std::regex re(
                R"(String\s*\.\s*from(?:CharCode|CodePoint)\s*\(\s*([0-9a-fA-FxX,\s]+)\s*\))");

            std::regex numRe(R"((?:0[xX][0-9a-fA-F]+)|(?:\d+))");

            std::string work = code, result;
            std::smatch m;
            int safety = 0;

            while (std::regex_search(work, m, re) && safety++ < 500) {
                result += m.prefix().str();

                std::string text;
                bool ok = true;
                auto b = std::sregex_iterator(m[1].first, m[1].second, numRe);
                auto e = std::sregex_iterator();

                int count = 0;
                for (auto it = b; it != e; ++it) {
                    uint32_t v;
                    if (!parseNum((*it)[0], v)) { ok = false; break; }
                    if (v > 0x10FFFF) { ok = false; break; }
                    appendUtf8(text, v);
                    ++count;
                }

                if (ok && count > 0 && looksLikeText(text) && isValidUtf8(text)) {
                    result += "\"" + escapeJs(text) + "\"";
                    Logger::debug("  [js-charcode] -> \"" + text + "\"");
                    changed = true;
                }
                else {
                    result += m[0].str();
                }
                work = m.suffix().str();
            }
            result += work;
            code = result;

            if (!changed) Logger::debug("  [js-charcode] fromCharCode не найдено");
            return changed;
        }

    }
} // namespace