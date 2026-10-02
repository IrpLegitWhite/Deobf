#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class PythonBase64VarsPass : public Pass {
        public:
            std::string name() const override { return "python-base64-vars"; }
            std::string description() const override {
                return "Раскрывает base64.b64decode(var) если var = \"base64string\"";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf