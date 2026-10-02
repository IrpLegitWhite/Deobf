#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class IdentifiersPass : public Pass {
        public:
            std::string name() const override { return "identifiers"; }
            std::string description() const override {
                return "Переименовывает обфусцированные идентификаторы (_0x1a2b, __xxx, l1l1l)";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf