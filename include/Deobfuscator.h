#pragma once

#include "core/Config.h"
#include "passes/Pass.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace deobf {

    class Deobfuscator {
    public:
        explicit Deobfuscator(const core::Config& cfg);
        ~Deobfuscator() = default;

        void run();

    private:
        void registerPasses();

        void reg(const std::string& name,
            std::function<std::shared_ptr<passes::Pass>()> factory);

        core::Config cfg_;

        std::unordered_map<
            std::string,
            std::function<std::shared_ptr<passes::Pass>()>
        > factories_;
    };

} // namespace deobf