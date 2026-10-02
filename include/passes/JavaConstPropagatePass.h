#pragma once
#include "Pass.h"

namespace deobf {
    namespace passes {

        class JavaConstPropagatePass : public Pass {
        public:
            std::string name() const override { return "java-const-propagate"; }
            std::string description() const override {
                return "final String X = ...; X -> \"...\"";
            }

            // ★ Применяем только если есть final String объявления
            bool canApply(const std::string& code) const override {
                return code.find("final String ") != std::string::npos ||
                    code.find("final  String ") != std::string::npos;
            }

            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf