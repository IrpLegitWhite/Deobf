#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class JsDeadFuncPass : public Pass {
        public:
            std::string name() const override { return "js-funcs"; }
            std::string description() const override {
                return "Удаляет мёртвые JS-функции (_dead*, _trap*)";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf