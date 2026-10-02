#pragma once
#include "Pass.h"

namespace deobf {
    namespace passes {

        class JavaStringXorPass : public Pass {
        public:
            std::string name() const override { return "java-string-xor"; }
            std::string description() const override {
                return "(byte)(A ^ B) / new byte[]{...} -> string";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf