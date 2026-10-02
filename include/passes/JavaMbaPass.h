#pragma once
#include "Pass.h"

namespace deobf {
    namespace passes {

        class JavaMbaPass : public Pass {
        public:
            std::string name() const override { return "java-mba"; }
            std::string description() const override {
                return "x*1, x/1, x|0, x^0 -> x";
            }

            bool canApply(const std::string& code) const override {
                // Применяем только если есть "* 1", "/ 1", "| 0", "^ 0"
                return code.find("* 1") != std::string::npos ||
                    code.find("/ 1") != std::string::npos ||
                    code.find("| 0") != std::string::npos ||
                    code.find("^ 0") != std::string::npos;
            }

            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf