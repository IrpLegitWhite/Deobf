#pragma once
#include "lang/Language.h"

namespace deobf {
    namespace lang {

        class CppLang : public Language {
        public:
            std::string id() const override { return "cpp"; }
            std::vector<std::string> extensions() const override {
                return { ".cpp", ".cc", ".cxx", ".c++", ".hpp", ".hh", ".hxx" };
            }
            std::vector<std::string> defaultPasses() const override {
                return {
                    "define-expand", "xor-strings", "decoder", "chr-chains",
                    "split-strings", "char-math", "const-fold", "junk-ops",
                    "fake-branches", "identifiers", "rename", "dead-functions",
                    "empty-loops", "collapse-lines"
                };
            }
        };

    } // namespace lang
} // namespace deobf