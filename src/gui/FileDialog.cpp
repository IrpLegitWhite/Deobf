#include "../../include/gui/FileDialog.h"
#include <windows.h>
#include <commdlg.h>
#include <vector>
#include <string>

namespace deobf {
    namespace gui {

        static std::string wcharToUtf8(const wchar_t* w) {
            if (!w) return "";
            int len = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
            if (len <= 0) return "";
            std::string out(len - 1, '\0');
            WideCharToMultiByte(CP_UTF8, 0, w, -1, &out[0], len, nullptr, nullptr);
            return out;
        }

        std::string FileDialog::openFile(const wchar_t* title, const wchar_t* filter) {
            wchar_t buf[MAX_PATH] = { 0 };

            OPENFILENAMEW ofn = {};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = GetActiveWindow();
            ofn.lpstrFilter = filter;
            ofn.lpstrFile = buf;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrTitle = title;
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

            if (GetOpenFileNameW(&ofn)) return wcharToUtf8(buf);
            return "";
        }

        std::string FileDialog::saveFile(const wchar_t* title, const wchar_t* defaultExt,
            const wchar_t* filter) {
            wchar_t buf[MAX_PATH] = { 0 };

            OPENFILENAMEW ofn = {};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = GetActiveWindow();
            ofn.lpstrFilter = filter;
            ofn.lpstrFile = buf;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrTitle = title;
            ofn.lpstrDefExt = defaultExt;
            ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

            if (GetSaveFileNameW(&ofn)) return wcharToUtf8(buf);
            return "";
        }

    } // namespace gui
} // namespace deobf