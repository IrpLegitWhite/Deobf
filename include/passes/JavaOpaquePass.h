#pragma once
#include "Pass.h"

namespace deobf {
    namespace passes {

        class JavaOpaquePass : public Pass {
        public:
            std::string name() const override { return "java-opaque"; }
            std::string description() const override {
                return "(x*x>=0) -> true, opaque predicates";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf