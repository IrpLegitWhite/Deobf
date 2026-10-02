#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class DeadPythonFuncPass : public Pass {
        public:
            std::string name() const override { return "dead-python-funcs"; }
            std::string description() const override {
                return "Удаляет мёртвые функции Python (_dead*, _trap*)";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf