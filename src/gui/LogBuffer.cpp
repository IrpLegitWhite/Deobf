#include "../../include/gui/LogBuffer.h"

namespace deobf {
    namespace gui {

        LogBuffer& LogBuffer::instance() {
            static LogBuffer buf;
            return buf;
        }

        void LogBuffer::push(const std::string& line) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_lines.push_back(line);
            if (m_lines.size() > 500) {
                m_lines.erase(m_lines.begin(), m_lines.begin() + 100);
            }
        }

        std::vector<std::string> LogBuffer::snapshot() {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_lines;
        }

        void LogBuffer::clear() {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_lines.clear();
        }

    } // namespace gui
} // namespace deobf