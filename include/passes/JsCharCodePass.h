#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class JsCharCodePass : public Pass {
        public:
            std::string name() const override { return "js-charcode"; }
            std::string description() const override {
                return "Собирает String.fromCharCode(72, 101, ...) в строку";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf