#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class EmptyLoopPass : public Pass {
        public:
            std::string name() const override { return "empty-loops"; }
            std::string description() const override {
                return "Удаляет мусорные циклы вида for(int i=0;i<1;++i) ...";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf