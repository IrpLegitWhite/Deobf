#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class JsXorPass : public Pass {
        public:
            std::string name() const override { return "js-xor"; }
            std::string description() const override {
                return "Раскрывает XOR-циклы JS";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf