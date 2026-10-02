#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class ReverseStringsPass : public Pass {
        public:
            std::string name() const override { return "reverse-strings"; }
            std::string description() const override {
                return "Разворачивает \"...\"[::-1] в обычную строку";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf