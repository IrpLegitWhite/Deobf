#pragma once
#include <string>

namespace deobf {
    namespace passes {

        class Pass {
        public:
            virtual ~Pass() = default;
            virtual std::string name() const = 0;
            virtual std::string description() const = 0;

            // ★ Опциональная проверка: применим ли пасс к этому коду?
            //   По умолчанию — true (всегда пытается).
            //   Пасс может переопределить и вернуть false, если паттернов нет.
            virtual bool canApply(const std::string& code) const {
                (void)code;
                return true;
            }

            virtual bool apply(std::string& code) = 0;
        };

    } // namespace passes
} // namespace deobf