#pragma once
#include <string>
#include <vector>

namespace deobf {
    namespace core {

        class File {
        public:
            static std::string read(const std::string& path);
            static void write(const std::string& path, const std::string& data);
            static std::vector<std::string> readLines(const std::string& path);
            static bool exists(const std::string& path);
            static std::string extension(const std::string& path);
        };

    } // namespace core
} // namespace deobf