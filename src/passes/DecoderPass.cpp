#include "../../include/passes/DecoderPass.h"
#include "../../include/core/Logger.h"
#include "../../include/core/Utils.h"

#include <string>
#include <cctype>
#include <cstdio>
#include <vector>

namespace deobf {
    namespace passes {

        using namespace deobf::core;

        static const std::string b64chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        static int b64val(char c) {
            size_t p = b64chars.find(c);
            if (p == std::string::npos) return -1;
            return static_cast<int>(p);
        }

        static bool tryBase64Decode(const std::string& in, std::string& out) {
            if (in.size() < 12 || in.size() % 4 != 0) return false;
            out.clear();
            for (size_t i = 0; i < in.size(); i += 4) {
                int v[4];
                for (int j = 0; j < 4; ++j) {
                    if (in[i + j] == '=') v[j] = -2;
                    else {
                        v[j] = b64val(in[i + j]);
                        if (v[j] < 0) return false;
                    }
                }
                int b1 = (v[0] << 2) | (v[1] >> 4);
                int b2 = ((v[1] & 0xF) << 4) | (v[2] >> 2);
                int b3 = ((v[2] & 0x3) << 6) | v[3];
                out.push_back(static_cast<char>(b1));
                if (v[2] != -2) out.push_back(static_cast<char>(b2));
                if (v[3] != -2) out.push_back(static_cast<char>(b3));
            }
            for (unsigned char c : out) {
                if (c == 0) return false;
                if (c >= 0x20 && c <= 0x7E) continue;
                if (c == '\n' || c == '\t') continue;
                if (c == 0xD0 || c == 0xD1) continue;
                if (c >= 0x80 && c <= 0xBF) continue;
                return false;
            }
            int letters = 0;
            for (unsigned char c : out) {
                if (std::isalnum(c) || c == ' ' || c == '.' || c == ',' ||
                    c == '!' || c == '?' || c == ':' || c == ';') letters++;
            }
            if (letters * 2 < (int)out.size()) return false;
            if (out == in) return false;
            return true;
        }

        static std::string decodeHexEscapes(const std::string& in) {
            std::string out;
            for (size_t i = 0; i < in.size(); ++i) {
                if (in[i] == '\\' && i + 3 < in.size() &&
                    (in[i + 1] == 'x' || in[i + 1] == 'X')) {
                    char hex[3] = { in[i + 2], in[i + 3], 0 };
                    try {
                        out.push_back(static_cast<char>(std::stoi(hex, nullptr, 16)));
                        i += 3;
                        continue;
                    }
                    catch (...) {}
                }
                out += in[i];
            }
            return out;
        }

        static std::string decodeUnicodeEscapes(const std::string& in) {
            std::string out;
            for (size_t i = 0; i < in.size(); ++i) {
                if (in[i] == '\\' && i + 5 < in.size() && in[i + 1] == 'u') {
                    char hex[5] = { in[i + 2], in[i + 3], in[i + 4], in[i + 5], 0 };
                    try {
                        unsigned int cp = static_cast<unsigned int>(std::stoul(hex, nullptr, 16));
                        if (cp < 0x80) {
                            out.push_back(static_cast<char>(cp));
                        }
                        else if (cp < 0x800) {
                            out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
                            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                        }
                        else {
                            out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
                            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                        }
                        i += 5;
                        continue;
                    }
                    catch (...) {}
                }
                out += in[i];
            }
            return out;
        }

        static bool isBase64Char(char c) {
            return (c >= 'A' && c <= 'Z') ||
                (c >= 'a' && c <= 'z') ||
                (c >= '0' && c <= '9') ||
                c == '+' || c == '/' || c == '=';
        }

        bool DecoderPass::apply(std::string& code) {
            bool changed = false;
            std::string out;
            out.reserve(code.size());
            size_t i = 0;
            const size_t N = code.size();

            while (i < N) {
                if (code[i] != '"') { out += code[i++]; continue; }

                size_t start = i;
                i++;
                std::string body;
                bool closed = false;
                while (i < N) {
                    if (code[i] == '\\' && i + 1 < N) {
                        body += code[i];
                        body += code[i + 1];
                        i += 2;
                        continue;
                    }
                    if (code[i] == '"') { closed = true; i++; break; }
                    if (code[i] == '\n') break;
                    body += code[i++];
                }

                if (!closed) {
                    out += code.substr(start, i - start);
                    continue;
                }

                std::string decoded = body;
                bool localChanged = false;

                if (decoded.find("\\x") != std::string::npos ||
                    decoded.find("\\X") != std::string::npos) {
                    std::string tmp = decodeHexEscapes(decoded);
                    if (tmp != decoded && utils::isPrintableUtf8(tmp)) {
                        decoded = tmp;
                        localChanged = true;
                        Logger::debug("  [decoder] hex -> \"" + decoded + "\"");
                    }
                }

                if (decoded.find("\\u") != std::string::npos) {
                    std::string tmp = decodeUnicodeEscapes(decoded);
                    if (tmp != decoded && utils::isPrintableUtf8(tmp)) {
                        decoded = tmp;
                        localChanged = true;
                        Logger::debug("  [decoder] unicode -> \"" + decoded + "\"");
                    }
                }

                if (!localChanged && decoded.size() >= 12 && decoded.size() % 4 == 0) {
                    bool allB64 = true;
                    for (char c : decoded) {
                        if (!isBase64Char(c)) { allB64 = false; break; }
                    }
                    if (allB64) {
                        std::string tmp;
                        if (tryBase64Decode(decoded, tmp)) {
                            decoded = tmp;
                            localChanged = true;
                            Logger::debug("  [decoder] base64 -> \"" + decoded + "\"");
                        }
                    }
                }

                out += '"';
                if (localChanged) {
                    out += utils::escapeCpp(decoded);
                    changed = true;
                }
                else {
                    out += body;
                }
                out += '"';
            }

            code = out;
            if (!changed) Logger::debug("  [decoder] нечего декодировать");
            return changed;
        }

    } // namespace passes
} // namespace deobf