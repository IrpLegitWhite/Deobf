#pragma once
#include "lang/Language.h"

namespace deobf {
    namespace lang {

        class JsLang : public Language {
        public:
            std::string id() const override { return "js"; }
            std::vector<std::string> extensions() const override {
                return { ".js", ".mjs", ".ts" };
            }
            std::vector<std::string> defaultPasses() const override {
                return {
                    "decoder", "reverse-strings", "split-strings",
                    "const-fold", "junk-ops", "double-negation",
                    "identifiers", "collapse-lines"
                };
            }
        };

    } // namespace lang
} // namespace deobf