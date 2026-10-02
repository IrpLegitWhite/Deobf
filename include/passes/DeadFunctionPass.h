#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class DeadFunctionPass : public Pass {
        public:
            std::string name() const override { return "dead-functions"; }
            std::string description() const override {
                return "Удаляет мёртвые функции (dead*, trap*, dummy*, stub*)";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf