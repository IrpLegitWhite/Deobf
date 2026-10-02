#pragma once
#include <string>

namespace deobf {
    namespace gui {

        class FileDialog {
        public:
            static std::string openFile(const wchar_t* title,
                const wchar_t* filter = L"Все файлы\0*.*\0");
            static std::string saveFile(const wchar_t* title,
                const wchar_t* defaultExt = L"cpp",
                const wchar_t* filter = L"Все файлы\0*.*\0");
        };

    } // namespace gui
} // namespace deobf