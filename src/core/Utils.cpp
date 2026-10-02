#include "../../include/core/Utils.h"
#include <algorithm>
#include <cctype>

namespace deobf {
    namespace core {
        namespace utils {

            std::string trim(const std::string& s) {
                size_t a = s.find_first_not_of(" \t\r\n");
                if (a == std::string::npos) return "";
                size_t b = s.find_last_not_of(" \t\r\n");
                return s.substr(a, b - a + 1);
            }

            std::vector<std::string> splitLines(const std::string& s) {
                std::vector<std::string> lines;
                std::string cur;
                for (char c : s) {
                    if (c == '\n') { lines.push_back(cur); cur.clear(); }
                    else if (c != '\r') cur += c;
                }
                if (!cur.empty()) lines.push_back(cur);
                return lines;
            }

            std::string join(const std::vector<std::string>& parts, const std::string& sep) {
                std::string out;
                for (size_t i = 0; i < parts.size(); ++i) {
                    if (i) out += sep;
                    out += parts[i];
                }
                return out;
            }

            bool isPrintableUtf8(const std::string& s) {
                for (unsigned char c : s) {
                    if (c == 0) continue;
                    if (c >= 0x20 && c <= 0x7E) continue;
                    if (c >= 0x80) continue;
                    return false;
                }
                return true;
            }

            std::string escapeCpp(const std::string& s) {
                std::string out;
                out.reserve(s.size() * 2);
                for (char c : s) {
                    if (c == '"' || c == '\\') out += '\\';
                    out += c;
                }
                return out;
            }

            std::string toLower(std::string s) {
                for (auto& c : s) {
                    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                }
                return s;
            }

            std::string toUpper(std::string s) {
                for (auto& c : s) {
                    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                }
                return s;
            }

            bool startsWith(const std::string& s, const std::string& prefix) {
                if (prefix.size() > s.size()) return false;
                return std::equal(prefix.begin(), prefix.end(), s.begin());
            }

            bool endsWith(const std::string& s, const std::string& suffix) {
                if (suffix.size() > s.size()) return false;
                return std::equal(suffix.rbegin(), suffix.rend(), s.rbegin());
            }

            std::string replaceAll(std::string s, const std::string& from, const std::string& to) {
                if (from.empty()) return s;
                size_t pos = 0;
                while ((pos = s.find(from, pos)) != std::string::npos) {
                    s.replace(pos, from.size(), to);
                    pos += to.size();
                }
                return s;
            }

        } // namespace utils
    } // namespace core
} // namespace deobf