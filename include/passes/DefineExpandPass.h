#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class DefineExpandPass : public Pass {
        public:
            std::string name() const override { return "define-expand"; }
            std::string description() const override {
                return "Разворачивает #define-макросы в их значения";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf