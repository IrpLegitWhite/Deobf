#include "../include/Deobfuscator.h"
#include "../include/core/Logger.h"
#include "../include/core/File.h"

// ---- C/C++/C# ----
#include "../include/passes/DefineExpandPass.h"
#include "../include/passes/XorStringPass.h"
#include "../include/passes/DecoderPass.h"
#include "../include/passes/SplitStringPass.h"
#include "../include/passes/CharMathPass.h"
#include "../include/passes/ConstFoldPass.h"
#include "../include/passes/JunkOpPass.h"
#include "../include/passes/DoubleNegationPass.h"
#include "../include/passes/OpaquePredicatePass.h"
#include "../include/passes/IdentifiersPass.h"
#include "../include/passes/IdentifierRenamePass.h"
#include "../include/passes/DeadFunctionPass.h"
#include "../include/passes/EmptyLoopPass.h"
#include "../include/passes/CollapseLinesPass.h"
#include "../include/passes/FakeBranchPass.h"

// ---- Python ----
#include "../include/passes/ChrChainsPass.h"
#include "../include/passes/PythonBase64Pass.h"
#include "../include/passes/PythonBase64VarsPass.h"
#include "../include/passes/PythonXorMultiPass.h"
#include "../include/passes/PythonXorPass.h"
#include "../include/passes/DeadPythonFuncPass.h"
#include "../include/passes/ReverseStringsPass.h"

// ---- JavaScript ----
#include "../include/passes/JsAtobPass.h"
#include "../include/passes/JsCharCodePass.h"
#include "../include/passes/JsReversePass.h"
#include "../include/passes/JsXorPass.h"
#include "../include/passes/JsDeadFuncPass.h"

// ---- Java ----
#include "../include/passes/JavaUnicodeEscapePass.h"
#include "../include/passes/JavaConstPropagatePass.h"
#include "../include/passes/JavaBase64Pass.h"
#include "../include/passes/JavaCharCodePass.h"
#include "../include/passes/JavaReversePass.h"
#include "../include/passes/JavaStringXorPass.h"
#include "../include/passes/JavaDeadCodePass.h"
#include "../include/passes/JavaOpaquePass.h"
#include "../include/passes/JavaMbaPass.h"
#include "../include/passes/JavaIdentifiersPass.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace deobf {

    using namespace deobf::core;

    Deobfuscator::Deobfuscator(const Config& cfg) : cfg_(cfg) {
        registerPasses();
    }

    void Deobfuscator::registerPasses() {
        // ---------- C/C++/C# ----------
        reg("define-expand", [] { return std::make_shared<passes::DefineExpandPass>(); });
        reg("xor-strings", [] { return std::make_shared<passes::XorStringPass>(); });
        reg("decoder", [] { return std::make_shared<passes::DecoderPass>(); });
        reg("split-strings", [] { return std::make_shared<passes::SplitStringPass>(); });
        reg("char-math", [] { return std::make_shared<passes::CharMathPass>(); });
        reg("const-fold", [] { return std::make_shared<passes::ConstFoldPass>(); });
        reg("junk-ops", [] { return std::make_shared<passes::JunkOpPass>(); });
        reg("double-negation", [] { return std::make_shared<passes::DoubleNegationPass>(); });
        reg("opaque-predicates", [] { return std::make_shared<passes::OpaquePredicatePass>(); });
        reg("identifiers", [] { return std::make_shared<passes::IdentifiersPass>(); });
        reg("rename", [] { return std::make_shared<passes::IdentifierRenamePass>(); });
        reg("dead-functions", [] { return std::make_shared<passes::DeadFunctionPass>(); });
        reg("empty-loops", [] { return std::make_shared<passes::EmptyLoopPass>(); });
        reg("collapse-lines", [] { return std::make_shared<passes::CollapseLinesPass>(); });
        reg("fake-branch", [] { return std::make_shared<passes::FakeBranchPass>(); });

        // ---------- Python ----------
        reg("chr-chains", [] { return std::make_shared<passes::ChrChainsPass>(); });
        reg("python-base64", [] { return std::make_shared<passes::PythonBase64Pass>(); });
        reg("python-base64-vars", [] { return std::make_shared<passes::PythonBase64VarsPass>(); });
        reg("python-xor-multi", [] { return std::make_shared<passes::PythonXorMultiPass>(); });
        reg("python-xor", [] { return std::make_shared<passes::PythonXorPass>(); });
        reg("python-dead-funcs", [] { return std::make_shared<passes::DeadPythonFuncPass>(); });
        reg("reverse-strings", [] { return std::make_shared<passes::ReverseStringsPass>(); });

        // ---------- JavaScript ----------
        reg("js-atob", [] { return std::make_shared<passes::JsAtobPass>(); });
        reg("js-charcode", [] { return std::make_shared<passes::JsCharCodePass>(); });
        reg("js-reverse", [] { return std::make_shared<passes::JsReversePass>(); });
        reg("js-xor", [] { return std::make_shared<passes::JsXorPass>(); });
        reg("js-funcs", [] { return std::make_shared<passes::JsDeadFuncPass>(); });

        // ---------- Java ----------
        reg("java-unicode-escape", [] { return std::make_shared<passes::JavaUnicodeEscapePass>(); });
        reg("java-const-propagate", [] { return std::make_shared<passes::JavaConstPropagatePass>(); });
        reg("java-base64", [] { return std::make_shared<passes::JavaBase64Pass>(); });
        reg("java-charcode", [] { return std::make_shared<passes::JavaCharCodePass>(); });
        reg("java-reverse", [] { return std::make_shared<passes::JavaReversePass>(); });
        reg("java-string-xor", [] { return std::make_shared<passes::JavaStringXorPass>(); });
        reg("java-dead-code", [] { return std::make_shared<passes::JavaDeadCodePass>(); });
        reg("java-opaque", [] { return std::make_shared<passes::JavaOpaquePass>(); });
        reg("java-mba", [] { return std::make_shared<passes::JavaMbaPass>(); });
        reg("java-identifiers", [] { return std::make_shared<passes::JavaIdentifiersPass>(); });
    }

    void Deobfuscator::reg(const std::string& name,
        std::function<std::shared_ptr<passes::Pass>()> factory) {
        factories_[name] = std::move(factory);
    }

    void Deobfuscator::run() {
        Logger::info("Deobfuscator: start");
        Logger::info("  input : " + cfg_.inputPath);
        Logger::info("  output: " + cfg_.outputPath);
        Logger::info("  lang  : " + cfg_.language);

        std::string code;
        try {
            code = File::read(cfg_.inputPath);
        }
        catch (const std::exception& ex) {
            Logger::error(std::string("Не могу прочитать файл: ") + ex.what());
            return;
        }

        if (code.empty()) {
            Logger::error("Пустой файл");
            return;
        }

        Logger::info("  size  : " + std::to_string(code.size()) + " байт");

        const int MAX_ROUNDS = 10;
        int round = 0;

        while (round < MAX_ROUNDS) {
            ++round;
            bool changedThisRound = false;
            Logger::info("── Раунд " + std::to_string(round) + " ──");

            for (const auto& passName : cfg_.enabledPasses) {
                auto it = factories_.find(passName);
                if (it == factories_.end()) {
                    if (cfg_.verbose)
                        Logger::debug("  [skip] неизвестный пасс: " + passName);
                    continue;
                }

                auto pass = it->second();

                // ★ ПРОВЕРКА: применим ли пасс к текущему коду?
                if (!pass->canApply(code)) {
                    if (cfg_.verbose)
                        Logger::debug("  - " + passName + " not applicable (skip)");
                    continue;
                }

                std::string before = code;

                try {
                    bool changed = pass->apply(code);
                    if (changed && code != before) {
                        changedThisRound = true;
                        Logger::info("  OK " + passName);
                    }
                    else if (cfg_.verbose) {
                        Logger::debug("  - " + passName + " no change");
                    }
                }
                catch (const std::exception& ex) {
                    Logger::error("  FAIL " + passName + ": " + ex.what());
                    code = before;   // ★ откат при ошибке
                }
                catch (...) {
                    Logger::error("  FAIL " + passName + ": unknown error");
                    code = before;
                }
            }

            if (!changedThisRound) {
                Logger::info("Больше изменений нет, стоп.");
                break;
            }
        }

        if (round >= MAX_ROUNDS)
            Logger::info("Достигнут максимум раундов (" + std::to_string(MAX_ROUNDS) + ")");

        if (cfg_.dryRun) {
            Logger::info("Dry-run: запись пропущена");
        }
        else {
            try {
                File::write(cfg_.outputPath, code);
                Logger::info("Записано: " + cfg_.outputPath + " (" +
                    std::to_string(code.size()) + " байт)");
            }
            catch (const std::exception& ex) {
                Logger::error(std::string("Не могу записать: ") + ex.what());
            }
        }

        Logger::info("Deobfuscator: готово");
    }

} // namespace deobf