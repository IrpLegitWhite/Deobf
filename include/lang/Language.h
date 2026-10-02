#pragma once
#include <string>
#include <vector>

namespace deobf {
    namespace lang {

        class Language {
        public:
            virtual ~Language() = default;
            virtual std::string id() const = 0;
            virtual std::vector<std::string> extensions() const = 0;
            virtual std::vector<std::string> defaultPasses() const = 0;
        };

    } // namespace lang
} // namespace deobf