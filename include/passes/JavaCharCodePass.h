#pragma once
#include "Pass.h"

namespace deobf {
    namespace passes {

        class JavaCharCodePass : public Pass {
        public:
            std::string name() const override { return "java-charcode"; }
            std::string description() const override {
                return "new String(new char[]{72,101,...}) -> \"Hello\"";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf