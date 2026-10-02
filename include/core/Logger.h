#pragma once
#include <string>

namespace deobf {
    namespace core {

        enum class LogLevel {
            Debug = 0,
            Info = 1,
            Warn = 2,
            Error = 3
        };

        class Logger {
        public:
            static void setLevel(LogLevel lvl);
            static void log(LogLevel lvl, const std::string& msg);

            static void debug(const std::string& msg);
            static void info(const std::string& msg);
            static void warn(const std::string& msg);
            static void error(const std::string& msg);

        private:
            static LogLevel s_level;
        };

    } // namespace core
} // namespace deobf