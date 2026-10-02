#include "../../include/passes/IdentifiersPass.h"
#include "../../include/core/Logger.h"
#include "../../include/core/Utils.h"

#include <set>
#include <map>
#include <regex>
#include <string>
#include <cctype>

namespace deobf {
    namespace passes {

        using namespace deobf::core;

        static bool isReserved(const std::string& name) {
            static const std::set<std::string> kw = {
                "int","long","short","char","float","double","void","bool","signed","unsigned",
                "const","static","volatile","extern","register","inline","virtual","explicit",
                "class","struct","union","enum","namespace","using","typedef","template",
                "typename","public","private","protected","friend","operator","new","delete",
                "this","nullptr","true","false","if","else","for","while","do","switch",
                "case","default","break","continue","return","goto","try","catch","throw",
                "sizeof","alignof","typeid","std","cout","cin","cerr","endl","string","vector",
                "map","set","pair","size_t","uint8_t","uint16_t","uint32_t","uint64_t",
                "int8_t","int16_t","int32_t","int64_t","main","printf","scanf","malloc",
                "free","memcpy","memset","strlen","strcmp","strcpy","NULL",
                "def","lambda","import","from","as","with","yield","global","nonlocal",
                "pass","raise","finally","assert","del","elif","not","is","None","True","False",
                "print","range","len","str","list","dict","tuple","set","open","self",
                "bytes","bytearray","int","float","bool","type","class","object",
                "base64","b64decode","b64encode","zlib","marshal","pickle","codecs",
                "exec","eval","compile","getattr","setattr","hasattr","chr","ord",
                "join","split","replace","strip","format","encode","decode",
                "__import__","__init__","__main__","__name__","__file__","__dict__",
                "__class__","__doc__","__module__","__call__","__str__","__repr__",
            };
            return kw.count(name) > 0;
        }

        static bool looksObfuscated(const std::string& name) {
            if (name.size() < 4) return false;

            if (name.size() >= 2 && name[0] == '_' && name[1] == '_') {
                bool isHexAfter = (name.size() > 3 && name[2] == '0' &&
                    (name[3] == 'x' || name[3] == 'X'));
                if (!isHexAfter) return false;
            }
            if (name.size() > 3 && name[0] == '_' && name[1] == '0' &&
                (name[2] == 'x' || name[2] == 'X')) return true;

            bool conf = true;
            for (char c : name) {
                if (c != 'l' && c != 'I' && c != '1' && c != 'O' && c != '0') {
                    conf = false; break;
                }
            }
            if (conf && name.size() >= 5) return true;

            if (name.size() >= 9 && std::isalpha((unsigned char)name[0])) {
                bool allHex = true;
                for (size_t i = 1; i < name.size(); ++i) {
                    if (!std::isxdigit((unsigned char)name[i])) { allHex = false; break; }
                }
                if (allHex) return true;
            }

            if (name.size() >= 8 && name[0] == '_' &&
                std::isxdigit((unsigned char)name[1])) {
                bool allHex = true;
                for (size_t i = 1; i < name.size(); ++i) {
                    if (!std::isxdigit((unsigned char)name[i])) { allHex = false; break; }
                }
                if (allHex && name.size() >= 8) return true;
            }
            return false;
        }

        bool IdentifiersPass::apply(std::string& code) {
            std::regex wordRe(R"(\b([A-Za-z_][A-Za-z0-9_]{3,})\b)");
            std::set<std::string> candidates;

            std::sregex_iterator it(code.begin(), code.end(), wordRe);
            std::sregex_iterator endIt;

            while (it != endIt) {
                std::string name = (*it)[1];
                if (!isReserved(name) && looksObfuscated(name)) {
                    candidates.insert(name);
                }
                ++it;
            }

            if (candidates.empty()) {
                Logger::debug("  [identifiers] обфусцированных имён не найдено");
                return false;
            }

            int funcCounter = 1;
            int varCounter = 1;
            std::map<std::string, std::string> renameMap;

            for (const auto& name : candidates) {
                std::regex funcRe("\\b" + name + "\\s*\\(");
                bool isFunc = std::regex_search(code, funcRe);
                if (isFunc) renameMap[name] = "fn_" + std::to_string(funcCounter++);
                else        renameMap[name] = "var_" + std::to_string(varCounter++);
            }

            bool changed = false;
            for (const auto& kv : renameMap) {
                const std::string& from = kv.first;
                const std::string& to = kv.second;
                std::string before = code;
                size_t pos = 0;
                while ((pos = code.find(from, pos)) != std::string::npos) {
                    bool leftOk = (pos == 0) ||
                        !(std::isalnum((unsigned char)code[pos - 1]) || code[pos - 1] == '_');
                    size_t endp = pos + from.size();
                    bool rightOk = (endp >= code.size()) ||
                        !(std::isalnum((unsigned char)code[endp]) || code[endp] == '_');
                    if (leftOk && rightOk) {
                        code.replace(pos, from.size(), to);
                        pos += to.size();
                    }
                    else {
                        pos += from.size();
                    }
                }
                if (code != before) {
                    Logger::debug("  [identifiers] " + from + " -> " + to);
                    changed = true;
                }
            }
            return changed;
        }

    } // namespace passes
} // namespace deobf