#include "../../include/core/Logger.h"
#include "../../include/gui/LogBuffer.h"

#include <iostream>

namespace deobf {
    namespace core {

        LogLevel Logger::s_level = LogLevel::Info;

        void Logger::setLevel(LogLevel lvl) {
            s_level = lvl;
        }

        static const char* lvlName(LogLevel lvl) {
            switch (lvl) {
            case LogLevel::Debug: return "DEBUG";
            case LogLevel::Info:  return "INFO";
            case LogLevel::Warn:  return "WARN";
            case LogLevel::Error: return "ERROR";
            }
            return "?";
        }

        void Logger::log(LogLevel lvl, const std::string& msg) {
            if (static_cast<int>(lvl) < static_cast<int>(s_level)) return;

            std::string line = std::string("[") + lvlName(lvl) + "] " + msg;
            std::cerr << line << "\n";

            deobf::gui::LogBuffer::instance().push(line);
        }

        void Logger::debug(const std::string& m) { log(LogLevel::Debug, m); }
        void Logger::info(const std::string& m) { log(LogLevel::Info, m); }
        void Logger::warn(const std::string& m) { log(LogLevel::Warn, m); }
        void Logger::error(const std::string& m) { log(LogLevel::Error, m); }

    } // namespace core
} // namespace deobf