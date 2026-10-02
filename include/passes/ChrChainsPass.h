#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class ChrChainsPass : public Pass {
        public:
            std::string name() const override { return "chr-chains"; }
            std::string description() const override {
                return "Собирает chr(N)+chr(M)+... и [chr(c) for c in [...]] в строку";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf