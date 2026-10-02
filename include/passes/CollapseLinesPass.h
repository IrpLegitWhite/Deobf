#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class CollapseLinesPass : public Pass {
        public:
            std::string name() const override { return "collapse-lines"; }
            std::string description() const override {
                return "Сжимает пустые строки и убирает висячие пробелы";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf