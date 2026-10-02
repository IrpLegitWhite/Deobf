#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class PythonBase64Pass : public Pass {
        public:
            std::string name() const override { return "python-base64"; }
            std::string description() const override {
                return "Раскрывает base64.b64decode(\"...\").decode() в строку";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf