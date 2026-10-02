#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class XorStringPass : public Pass {
        public:
            std::string name() const override { return "xor-strings"; }
            std::string description() const override {
                return "Расшифровывает XOR-строки (unsigned char NAME[] = {...})";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf
