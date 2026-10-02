#pragma once
#include "lang/Language.h"

namespace deobf {
    namespace lang {

        class CLang : public Language {
        public:
            std::string id() const override { return "c"; }
            std::vector<std::string> extensions() const override {
                return { ".c", ".h" };
            }
            std::vector<std::string> defaultPasses() const override {
                return {
                    "define-expand", "xor-strings", "dead-functions",
                    "empty-loops", "rename", "collapse-lines"
                };
            }
        };

    } // namespace lang
} // namespace deobf