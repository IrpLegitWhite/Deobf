#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class SplitStringPass : public Pass {
        public:
            std::string name() const override { return "split-strings"; }
            std::string description() const override {
                return "Склеивает разбитые строки \"abc\" \"def\" -> \"abcdef\"";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf