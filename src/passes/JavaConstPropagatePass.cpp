#include "../../include/passes/JavaConstPropagatePass.h"
#include "../../include/core/Logger.h"

#include <regex>
#include <string>
#include <unordered_map>
#include <cctype>

namespace deobf {
    namespace passes {

        using namespace deobf::core;

        static bool isIdentChar(char c) {
            return std::isalnum((unsigned char)c) || c == '_';
        }
        static bool isIdentStart(char c) {
            return std::isalpha((unsigned char)c) || c == '_';
        }

        bool JavaConstPropagatePass::apply(std::string& code) {
            bool changed = false;

            // Regex для объявления final String
            std::regex declRe(
                "(?:^|[^A-Za-z0-9_])(?:static\\s+)?final\\s+String\\s+"
                "(\\w+)\\s*=\\s*\"((?:[^\"\\\\]|\\\\.)*)\"\\s*;");

            std::unordered_map<std::string, std::string> consts;

            {
                auto begin = std::sregex_iterator(code.begin(), code.end(), declRe);
                auto end = std::sregex_iterator();
                for (auto it = begin; it != end; ++it) {
                    std::string name = (*it)[1].str();
                    std::string value = (*it)[2].str();
                    if (consts.count(name)) continue;

                    std::regex redef("(?:^|[^A-Za-z0-9_])String\\s+" + name + "\\s*=[^=]");
                    int count = 0;
                    auto b2 = std::sregex_iterator(code.begin(), code.end(), redef);
                    for (auto it2 = b2; it2 != end; ++it2) count++;

                    if (count == 1) consts[name] = value;
                }
            }

            if (consts.empty()) {
                Logger::debug("  [java-const] no final constants found");
                return false;
            }

            Logger::debug("  [java-const] found " + std::to_string(consts.size()) + " constants");

            std::string result;
            result.reserve(code.size() + 256);

            size_t i = 0;
            bool inStr = false, inChar = false, inLineCmt = false, inBlockCmt = false;

            while (i < code.size()) {
                char c = code[i];
                char n = (i + 1 < code.size()) ? code[i + 1] : '\0';

                if (inLineCmt) { result += c; if (c == '\n') inLineCmt = false; ++i; continue; }
                if (inBlockCmt) {
                    result += c;
                    if (c == '*' && n == '/') { result += n; i += 2; inBlockCmt = false; continue; }
                    ++i; continue;
                }
                if (inStr) {
                    result += c;
                    if (c == '\\' && i + 1 < code.size()) { result += code[i + 1]; i += 2; continue; }
                    if (c == '"') inStr = false;
                    ++i; continue;
                }
                if (inChar) {
                    result += c;
                    if (c == '\\' && i + 1 < code.size()) { result += code[i + 1]; i += 2; continue; }
                    if (c == '\'') inChar = false;
                    ++i; continue;
                }

                if (c == '"') { inStr = true;       result += c; ++i; continue; }
                if (c == '\'') { inChar = true;      result += c; ++i; continue; }
                if (c == '/' && n == '/') { inLineCmt = true;  result += c; ++i; continue; }
                if (c == '/' && n == '*') { inBlockCmt = true; result += c; ++i; continue; }

                if (isIdentStart(c)) {
                    size_t start = i;
                    while (i < code.size() && isIdentChar(code[i])) ++i;
                    std::string word = code.substr(start, i - start);

                    auto it = consts.find(word);
                    if (it == consts.end()) { result += word; continue; }

                    // ★★ ЗАМЕНЯЕМ ТОЛЬКО ЕСЛИ ЭТО "USE", А НЕ "DECL".
                    //   Критерий: смотрим на СЛЕДУЮЩИЙ не-пробельный символ.
                    //   - Если '=' → это объявление (NAME = ...), НЕ заменяем.
                    //   - Если ';' или ',' или ')' → это объявление или конец, НЕ заменяем.
                    //   - Иначе → заменяем.
                    size_t j = i;
                    while (j < code.size() && std::isspace((unsigned char)code[j])) ++j;
                    char next = (j < code.size()) ? code[j] : '\0';

                    if (next == '=' && !(j + 1 < code.size() && code[j + 1] == '=')) {
                        // Объявление "NAME = ..." — не заменяем
                        result += word;
                        Logger::debug("  [java-const] skip decl: " + word);
                        continue;
                    }

                    result += "\"";
                    result += it->second;
                    result += "\"";
                    changed = true;
                    Logger::debug("  [java-const] replace: " + word + " -> \"" + it->second + "\"");
                    continue;
                }

                result += c;
                ++i;
            }

            if (changed) code = result;
            if (!changed) Logger::debug("  [java-const] no replacements");
            return changed;
        }

    } // namespace passes
} // namespace deobf