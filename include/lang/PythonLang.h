#pragma once
#include "lang/Language.h"

namespace deobf {
    namespace lang {

        class PythonLang : public Language {
        public:
            std::string id() const override { return "python"; }
            std::vector<std::string> extensions() const override {
                return { ".py", ".pyw" };
            }
            std::vector<std::string> defaultPasses() const override {
                return {
                    "decoder", "chr-chains", "python-base64", "python-xor",
                    "split-strings", "const-fold", "junk-ops", "double-negation",
                    "opaque-predicates", "identifiers", "collapse-lines"
                };
            }
        };

    } // namespace lang
} // namespace deobf