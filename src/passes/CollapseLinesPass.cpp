#include "../../include/passes/CollapseLinesPass.h"
#include "../../include/core/Logger.h"
#include "../../include/core/Utils.h"

#include <sstream>
#include <string>

namespace deobf {
    namespace passes {

        using namespace deobf::core;

        bool CollapseLinesPass::apply(std::string& code) {
            std::string before = code;

            auto lines = utils::splitLines(code);
            std::stringstream out;

            int emptyRun = 0;
            for (const auto& line : lines) {
                std::string trimmed = line;
                while (!trimmed.empty() &&
                    (trimmed.back() == ' ' || trimmed.back() == '\t')) {
                    trimmed.pop_back();
                }
                if (trimmed.empty()) {
                    emptyRun++;
                    if (emptyRun <= 1) out << "\n";
                    continue;
                }
                emptyRun = 0;
                out << trimmed << "\n";
            }

            std::string result = out.str();
            while (!result.empty() &&
                (result.back() == '\n' || result.back() == ' ' || result.back() == '\t')) {
                result.pop_back();
            }
            result += "\n";

            code = result;

            bool changed = (code != before);
            if (changed) Logger::debug("  [collapse-lines] строки сжаты");
            else         Logger::debug("  [collapse-lines] нечего сжимать");

            return changed;
        }

    } // namespace passes
} // namespace deobf