#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class JsReversePass : public Pass {
        public:
            std::string name() const override { return "js-reverse"; }
            std::string description() const override {
                return "Разворачивает \"...\".split('').reverse().join('')";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf