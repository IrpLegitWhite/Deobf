#pragma once
#include "Pass.h"

namespace deobf {
    namespace passes {

        class JavaIdentifiersPass : public Pass {
        public:
            std::string name() const override { return "java-identifiers"; }
            std::string description() const override {
                return "_0x1a2b / lIlIlI -> var_N";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf