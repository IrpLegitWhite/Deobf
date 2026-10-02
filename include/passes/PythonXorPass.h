#pragma once
#include "passes/Pass.h"

namespace deobf {
    namespace passes {

        class PythonXorPass : public Pass {
        public:
            std::string name() const override { return "python-xor"; }
            std::string description() const override {
                return "Раскрывает XOR-циклы Python";
            }
            bool apply(std::string& code) override;
        };

    } // namespace passes
} // namespace deobf