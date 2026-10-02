#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class JsAtobPass : public Pass {
        public:
            std::string name() const override { return "js-atob"; }
            std::string description() const override {
                return "Раскрывает atob(\"base64\") в строку";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf