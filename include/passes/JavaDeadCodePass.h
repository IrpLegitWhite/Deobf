#pragma once
#include "Pass.h"

namespace deobf {
    namespace passes {

        class JavaDeadCodePass : public Pass {
        public:
            std::string name() const override { return "java-dead-code"; }
            std::string description() const override {
                return "if (false) {...} / if (true) {...}";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf