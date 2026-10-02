#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class DecoderPass : public Pass {
        public:
            std::string name() const override { return "decoder"; }
            std::string description() const override {
                return "Декодирует base64, \\xXX и \\uXXXX строки";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf