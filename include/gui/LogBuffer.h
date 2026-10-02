#pragma once
#include <string>
#include <mutex>
#include <vector>

namespace deobf {
    namespace gui {

        class LogBuffer {
        public:
            static LogBuffer& instance();
            void push(const std::string& line);
            std::vector<std::string> snapshot();
            void clear();

        private:
            LogBuffer() = default;
            std::mutex m_mutex;
            std::vector<std::string> m_lines;
        };

    } // namespace gui
} // namespace deobf