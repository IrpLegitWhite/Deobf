#include "../../include/passes/JavaIdentifiersPass.h"
#include "../../include/core/Logger.h"
#include <regex>
#include <string>
#include <set>
#include <unordered_map>

namespace deobf {
    namespace passes {

        using namespace deobf::core;

        bool JavaIdentifiersPass::apply(std::string& code) {
            bool changed = false;

            static const std::set<std::string> keywords = {
                "abstract","assert","boolean","break","byte","case","catch","char",
                "class","const","continue","default","do","double","else","enum",
                "extends","final","finally","float","for","goto","if","implements",
                "import","instanceof","int","interface","long","native","new","package",
                "private","protected","public","return","short","static","strictfp",
                "super","switch","synchronized","this","throw","throws","transient",
                "try","void","volatile","while","true","false","null",
                "String","Integer","Long","Double","Float","Boolean","Character",
                "Object","System","Math","Class","Override","Deprecated"
            };

            std::regex re("\\b(_0[xX][0-9a-fA-F]+|[lI1O0o]{6,})\\b");

            std::unordered_map<std::string, std::string> mapping;
            int counter = 1;

            std::string work = code;
            std::string result;
            std::smatch m;
            int safety = 0;

            while (std::regex_search(work, m, re) && safety++ < 5000) {
                result += m.prefix().str();
                std::string orig = m[1].str();

                if (keywords.count(orig)) {
                    result += orig;
                }
                else {
                    auto it = mapping.find(orig);
                    if (it == mapping.end()) {
                        std::string newName = "var_" + std::to_string(counter++);
                        mapping[orig] = newName;
                        changed = true;
                        result += newName;
                    }
                    else {
                        result += it->second;
                    }
                }
                work = m.suffix().str();
            }
            result += work;
            code = result;

            if (!changed) Logger::debug("  [java-ident] not found");
            return changed;
        }

    } // namespace passes
} // namespace deobf