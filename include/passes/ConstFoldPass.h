#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class ConstFoldPass : public Pass {
        public:
            std::string name() const override { return "const-fold"; }
            std::string description() const override {
                return "Сворачивает константные выражения: 2+3 -> 5";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf