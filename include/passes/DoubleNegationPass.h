#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class DoubleNegationPass : public Pass {
        public:
            std::string name() const override { return "double-negation"; }
            std::string description() const override {
                return "Убирает двойное отрицание: -(-(x)) → x, !(!(x)) → x";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf