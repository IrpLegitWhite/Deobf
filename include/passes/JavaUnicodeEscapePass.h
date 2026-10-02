#pragma once
#include "Pass.h"

namespace deobf {
    namespace passes {

        class JavaUnicodeEscapePass : public Pass {
        public:
            std::string name() const override { return "java-unicode-escape"; }
            std::string description() const override {
                return "\\uXXXX / \\uXXXX\\uXXXX -> UTF-8 string";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf