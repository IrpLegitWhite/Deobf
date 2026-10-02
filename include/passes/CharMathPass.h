#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class CharMathPass : public Pass {
        public:
            std::string name() const override { return "char-math"; }
            std::string description() const override {
                return "Собирает строки из char(N) + char(M) в литерал";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf