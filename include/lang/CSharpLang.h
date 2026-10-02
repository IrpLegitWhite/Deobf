#pragma once
#include "lang/Language.h"

namespace deobf {
    namespace lang {

        class CSharpLang : public Language {
        public:
            std::string id() const override { return "csharp"; }
            std::vector<std::string> extensions() const override {
                return { ".cs" };
            }
            std::vector<std::string> defaultPasses() const override {
                return {
                    "xor-strings", "decoder", "split-strings", "const-fold",
                    "junk-ops", "identifiers", "rename", "dead-functions",
                    "collapse-lines"
                };
            }
        };

    } // namespace lang
} // namespace deobf