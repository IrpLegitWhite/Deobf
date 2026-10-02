#pragma once
#include "Language.h"

namespace deobf {
    namespace lang {

        class JavaLang : public Language {
        public:
            std::string id() const override;
            std::vector<std::string> extensions() const override;
            std::vector<std::string> defaultPasses() const override;
        };

    } // namespace lang
} // namespace deobf