#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class JunkOpPass : public Pass {
        public:
            std::string name() const override { return "junk-ops"; }
            std::string description() const override {
                return "Убирает бесполезные операции: a+0, a*1, a|0, a^0";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf