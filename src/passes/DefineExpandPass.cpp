#include "../../include/passes/DefineExpandPass.h"
#include "../../include/core/Utils.h"
#include "../../include/core/Logger.h"

#include <regex>
#include <map>
#include <sstream>
#include <cctype>

namespace deobf {
    namespace passes {

        using namespace deobf::core;

        bool DefineExpandPass::apply(std::string& code) {
            std::map<std::string, std::string> defs;
            std::regex re(R"(^\s*#define\s+(\w+)\s+(.+?)\s*$)");

            for (const auto& line : utils::splitLines(code)) {
                std::smatch m;
                if (std::regex_match(line, m, re)) {
                    std::string defName = m[1];
                    std::string val = utils::trim(m[2]);
                    if (val.find('(') != std::string::npos) continue;
                    if (defName == "UNICODE" || defName == "_UNICODE") continue;
                    defs[defName] = val;
                }
            }

            if (defs.empty()) {
                Logger::debug("  [define-expand] макросов не найдено");
                return false;
            }

            std::stringstream out;
            for (const auto& line : utils::splitLines(code)) {
                std::smatch m;
                if (std::regex_match(line, m, re)) {
                    std::string defName = m[1];
                    if (defs.count(defName)) continue;
                }
                out << line << "\n";
            }
            code = out.str();

            for (const auto& kv : defs) {
                const std::string& defName = kv.first;
                const std::string& val = kv.second;

                std::string before = code;
                size_t pos = 0;
                while ((pos = code.find(defName, pos)) != std::string::npos) {
                    bool leftOk = (pos == 0) ||
                        !(std::isalnum((unsigned char)code[pos - 1]) || code[pos - 1] == '_');
                    size_t endp = pos + defName.size();
                    bool rightOk = (endp >= code.size()) ||
                        !(std::isalnum((unsigned char)code[endp]) || code[endp] == '_');
                    if (leftOk && rightOk) {
                        code.replace(pos, defName.size(), val);
                        pos += val.size();
                    }
                    else {
                        pos += defName.size();
                    }
                }
                if (code != before) {
                    Logger::debug("  [define-expand] " + defName + " -> " + val);
                }
            }

            return true;
        }

    } // namespace passes
} // namespace deobf