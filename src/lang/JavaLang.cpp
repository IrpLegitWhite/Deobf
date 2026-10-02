#include "../../include/lang/JavaLang.h"

namespace deobf {
    namespace lang {

        std::string JavaLang::id() const {
            return "java";
        }

        std::vector<std::string> JavaLang::extensions() const {
            return { ".java" };
        }

        std::vector<std::string> JavaLang::defaultPasses() const {
            return {
                "java-unicode-escape",
                "java-base64",
                "java-charcode",
                "java-reverse",
                "java-string-xor",
                "java-dead-code",
                "java-opaque",
                "java-identifiers",
            };
        }

    } // namespace lang
} // namespace deobf