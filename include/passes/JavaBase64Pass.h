#pragma once
#include "Pass.h"

namespace deobf {
    namespace passes {

        class JavaBase64Pass : public Pass {
        public:
            std::string name() const override { return "java-base64"; }
            std::string description() const override {
                return "new String(Base64.getDecoder().decode(\"...\")) -> \"...\"";
            }

            bool canApply(const std::string& code) const override {
                // Применяем только если есть "Base64" и "decode"
                return code.find("Base64") != std::string::npos &&
                    code.find("decode") != std::string::npos;
            }

            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf