#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class PythonXorMultiPass : public Pass {
        public:
            std::string name() const override { return "python-xor-multi"; }
            std::string description() const override {
                return "Сворачивает chr(ord(c) ^ K1 ^ K2) в chr(ord(c) ^ (K1^K2))";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf