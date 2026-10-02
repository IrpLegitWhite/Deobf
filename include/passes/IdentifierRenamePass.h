#pragma once
#include "passes/Pass.h"
#include <map>
#include <string>

namespace deobf {
    namespace passes {

        class IdentifierRenamePass : public Pass {
        public:
            explicit IdentifierRenamePass(const std::map<std::string, std::string>& extra = {})
                : m_extra(extra) {
            }

            std::string name() const override { return "rename"; }
            std::string description() const override {
                return "Переименовывает обусифицированные идентификаторы в читаемые";
            }
            bool apply(std::string& code) override;

        private:
            std::map<std::string, std::string> m_extra;
        };

    } // namespace passes
} // namespace deobf
