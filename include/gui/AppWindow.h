#pragma once
#include "TextEditor.h"

#include <string>
#include <thread>
#include <atomic>
#include <memory>

namespace deobf {
    namespace gui {

        class AppWindow {
        public:
            AppWindow();
            ~AppWindow();

            int run();

        private:
            void drawUI();
            void runDeobfuscatorAsync();

            std::string m_inputPath;
            std::string m_outputPath;
            std::string m_sourceText;
            std::string m_resultText;
            bool m_hasResult = false;
            bool m_needsEditorUpdate = false;

            int  m_languageIndex = 0;
            bool m_verbose = false;
            bool m_dryRun = false;

            std::thread m_worker;
            std::atomic<bool> m_running{ false };

            TextEditor m_editorLeft;
            TextEditor m_editorRight;
        };

    } // namespace gui
} // namespace deobf