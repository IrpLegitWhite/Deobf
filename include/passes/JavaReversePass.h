#pragma once
#include "Pass.h"

namespace deobf {
    namespace passes {

        class JavaReversePass : public Pass {
        public:
            std::string name() const override { return "java-reverse"; }
            std::string description() const override {
                return "new StringBuilder(\"...\").reverse().toString() -> \"...\"";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf