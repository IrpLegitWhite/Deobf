#include "../../include/core/File.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace deobf {
    namespace core {

        std::string File::read(const std::string& path) {
            std::ifstream f(path, std::ios::binary);
            if (!f) {
                throw std::runtime_error("File::read — не могу открыть: " + path);
            }
            std::stringstream ss;
            ss << f.rdbuf();
            return ss.str();
        }

        void File::write(const std::string& path, const std::string& data) {
            std::ofstream f(path, std::ios::binary);
            if (!f) {
                throw std::runtime_error("File::write — не могу записать: " + path);
            }
            f << data;
        }

        std::vector<std::string> File::readLines(const std::string& path) {
            auto content = read(path);
            std::vector<std::string> lines;
            std::string cur;
            for (char c : content) {
                if (c == '\n') { lines.push_back(cur); cur.clear(); }
                else if (c != '\r') cur += c;
            }
            if (!cur.empty()) lines.push_back(cur);
            return lines;
        }

        bool File::exists(const std::string& path) {
            return std::filesystem::exists(path);
        }

        std::string File::extension(const std::string& path) {
            auto p = std::filesystem::path(path);
            std::string ext = p.extension().string();
            for (auto& c : ext) {
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            }
            return ext;
        }

    } // namespace core
} // namespace deobf