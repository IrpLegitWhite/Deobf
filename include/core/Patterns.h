#pragma once
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

namespace deobf {
    namespace core {
        namespace patterns {

            enum class ObfType {
                Identifier,
                XorString,
                Base64String,
                HexString,
                UnicodeEscape,
                SplitString,
                CharMath,
                ConstFold,
                JunkOp,
                FakeBranch,
                DeadVariable,
                DeadFunction,
                EmptyLoop,
                StringArray,
            };

            struct Pattern {
                ObfType type;
                std::string name;
                std::string description;
            };

            inline const std::vector<Pattern>& all() {
                static const std::vector<Pattern> patterns = {
                    { ObfType::Identifier,    "hex-prefixed",     "_0x1a2b" },
                    { ObfType::Identifier,    "double-underscore","__0x1234" },
                    { ObfType::Identifier,    "confusing-chars",  "l1l1l1" },
                    { ObfType::XorString,     "xor-array",        "unsigned char[]" },
                    { ObfType::Base64String,  "base64-literal",   "SGVsbG8=" },
                    { ObfType::HexString,     "hex-escaped",      "\\xXX" },
                    { ObfType::UnicodeEscape, "unicode-escaped",  "\\uXXXX" },
                    { ObfType::SplitString,   "adjacent-literals","\"abc\" \"def\"" },
                    { ObfType::ConstFold,     "const-add",        "2 + 3" },
                    { ObfType::JunkOp,        "add-zero",         "a + 0" },
                    { ObfType::FakeBranch,    "if-true",          "if (1)" },
                    { ObfType::DeadFunction,  "dead-function",    "dead1()" },
                    { ObfType::EmptyLoop,     "empty-for-loop",   "for(i=0;i<1;++i)" },
                };
                return patterns;
            }

            inline bool isObfuscatedIdentifier(const std::string& name) {
                if (name.empty()) return false;
                if (name.size() > 3 && name[0] == '_' && name[1] == '0' &&
                    (name[2] == 'x' || name[2] == 'X')) return true;
                if (name.size() > 2 && name[0] == '_' && name[1] == '_') return true;
                bool conf = true;
                for (char c : name) {
                    if (c != 'l' && c != 'I' && c != '1' && c != 'O' && c != '0') {
                        conf = false; break;
                    }
                }
                if (conf && name.size() >= 5) return true;
                return false;
            }

            inline std::string typeName(ObfType t) {
                switch (t) {
                case ObfType::Identifier:     return "identifier";
                case ObfType::XorString:      return "xor-string";
                case ObfType::Base64String:   return "base64";
                case ObfType::HexString:      return "hex-string";
                case ObfType::UnicodeEscape:  return "unicode";
                case ObfType::SplitString:    return "split-string";
                case ObfType::CharMath:       return "char-math";
                case ObfType::ConstFold:      return "const-fold";
                case ObfType::JunkOp:         return "junk-op";
                case ObfType::FakeBranch:     return "fake-branch";
                case ObfType::DeadVariable:   return "dead-var";
                case ObfType::DeadFunction:   return "dead-func";
                case ObfType::EmptyLoop:      return "empty-loop";
                case ObfType::StringArray:    return "string-array";
                }
                return "unknown";
            }

        } // namespace patterns
    } // namespace core
} // namespace deobf