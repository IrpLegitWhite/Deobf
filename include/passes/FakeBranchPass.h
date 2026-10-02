#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class FakeBranchPass : public Pass {
        public:
            std::string name() const override { return "fake-branches"; }
            std::string description() const override {
                return "Убирает if (1), if (0), while (0) — мёртвые ветвления";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf