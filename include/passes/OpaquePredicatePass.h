#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class OpaquePredicatePass : public Pass {
        public:
            std::string name() const override { return "opaque-predicates"; }
            std::string description() const override {
                return "Убирает всегда-истинные условия: if x*x >= 0";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf