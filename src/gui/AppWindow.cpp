#define NOMINMAX

#include "../../include/gui/AppWindow.h"
#include "../../include/gui/FileDialog.h"
#include "../../include/gui/LogBuffer.h"
#include "../../include/core/Config.h"
#include "../../include/core/Logger.h"
#include "../../include/core/File.h"
#include "../../include/Deobfuscator.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "TextEditor.h"

#include <windows.h>
#include <d3d11.h>
#include <dwmapi.h>
#include <tchar.h>
#include <vector>
#include <string>
#include <cmath>
#include <cctype>
#include <cstdio>
#include <algorithm>

#pragma comment(lib, "dwmapi.lib")

// ============================================================
// DirectX 11 globals
// ============================================================
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

static void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (pBackBuffer) {
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }
}
static void CleanupRenderTarget() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}
static bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT flags = 0;
    D3D_FEATURE_LEVEL level;
    const D3D_FEATURE_LEVEL levels[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        flags, levels, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &level, &g_pd3dDeviceContext);
    if (hr == DXGI_ERROR_UNSUPPORTED)
        hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr,
            flags, levels, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &level, &g_pd3dDeviceContext);
    if (FAILED(hr)) return false;

    CreateRenderTarget();
    return true;
}
static void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;
    switch (msg) {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam),
                DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

static int countLines(const std::string& s) {
    if (s.empty()) return 0;
    int n = 1;
    for (char c : s) if (c == '\n') ++n;
    return n;
}

// ============================================================
// ПАЛИТРА
// ============================================================
namespace palette {
    const ImVec4 bgDeep = ImVec4(0.043f, 0.035f, 0.078f, 1.00f);
    const ImVec4 bgDark = ImVec4(0.071f, 0.059f, 0.118f, 1.00f);
    const ImVec4 bgMid = ImVec4(0.106f, 0.090f, 0.169f, 1.00f);
    const ImVec4 bgLight = ImVec4(0.149f, 0.125f, 0.235f, 1.00f);
    const ImVec4 bgHover = ImVec4(0.212f, 0.180f, 0.318f, 1.00f);
    const ImVec4 bgCard = ImVec4(0.094f, 0.078f, 0.149f, 1.00f);

    const ImVec4 accent = ImVec4(0.545f, 0.361f, 0.965f, 1.00f);
    const ImVec4 accentHot = ImVec4(0.702f, 0.533f, 1.000f, 1.00f);
    const ImVec4 accentDim = ImVec4(0.396f, 0.263f, 0.729f, 1.00f);

    const ImVec4 mint = ImVec4(0.259f, 0.898f, 0.784f, 1.00f);
    const ImVec4 mintHot = ImVec4(0.451f, 0.965f, 0.878f, 1.00f);

    const ImVec4 pink = ImVec4(1.000f, 0.361f, 0.612f, 1.00f);
    const ImVec4 pinkHot = ImVec4(1.000f, 0.541f, 0.741f, 1.00f);

    const ImVec4 cyan = ImVec4(0.302f, 0.788f, 0.980f, 1.00f);
    const ImVec4 cyanHot = ImVec4(0.478f, 0.878f, 1.000f, 1.00f);

    const ImVec4 orange = ImVec4(1.000f, 0.612f, 0.302f, 1.00f);
    const ImVec4 orangeHot = ImVec4(1.000f, 0.741f, 0.478f, 1.00f);

    const ImVec4 good = mint;
    const ImVec4 warn = ImVec4(1.000f, 0.780f, 0.310f, 1.00f);
    const ImVec4 err = ImVec4(1.000f, 0.361f, 0.478f, 1.00f);

    const ImVec4 textMain = ImVec4(0.957f, 0.949f, 0.988f, 1.00f);
    const ImVec4 textDim = ImVec4(0.663f, 0.639f, 0.749f, 1.00f);
    const ImVec4 textMute = ImVec4(0.427f, 0.404f, 0.510f, 1.00f);
}

// ============================================================
// ТЕМА ImGui
// ============================================================
static void ApplyCustomTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* c = style.Colors;

    style.WindowRounding = 12.0f;
    style.ChildRounding = 14.0f;
    style.FrameRounding = 12.0f;
    style.PopupRounding = 12.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding = 10.0f;
    style.TabRounding = 12.0f;

    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;

    style.WindowPadding = ImVec2(16, 16);
    style.FramePadding = ImVec2(14, 8);
    style.ItemSpacing = ImVec2(10, 10);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.IndentSpacing = 22.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 12.0f;

    using namespace palette;

    c[ImGuiCol_Text] = textMain;
    c[ImGuiCol_TextDisabled] = textMute;
    c[ImGuiCol_WindowBg] = bgDeep;
    c[ImGuiCol_ChildBg] = bgDark;
    c[ImGuiCol_PopupBg] = bgMid;
    c[ImGuiCol_Border] = ImVec4(accent.x, accent.y, accent.z, 0.25f);
    c[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_FrameBg] = bgLight;
    c[ImGuiCol_FrameBgHovered] = bgHover;
    c[ImGuiCol_FrameBgActive] = accentDim;
    c[ImGuiCol_TitleBg] = bgDeep;
    c[ImGuiCol_TitleBgActive] = bgMid;
    c[ImGuiCol_TitleBgCollapsed] = bgDeep;
    c[ImGuiCol_MenuBarBg] = bgDark;
    c[ImGuiCol_ScrollbarBg] = bgDeep;
    c[ImGuiCol_ScrollbarGrab] = bgHover;
    c[ImGuiCol_ScrollbarGrabHovered] = accentDim;
    c[ImGuiCol_ScrollbarGrabActive] = accent;
    c[ImGuiCol_CheckMark] = accentHot;
    c[ImGuiCol_SliderGrab] = accent;
    c[ImGuiCol_SliderGrabActive] = accentHot;
    c[ImGuiCol_Button] = bgLight;
    c[ImGuiCol_ButtonHovered] = bgHover;
    c[ImGuiCol_ButtonActive] = accentDim;
    c[ImGuiCol_Header] = bgLight;
    c[ImGuiCol_HeaderHovered] = bgHover;
    c[ImGuiCol_HeaderActive] = accentDim;
    c[ImGuiCol_Separator] = ImVec4(accent.x, accent.y, accent.z, 0.25f);
    c[ImGuiCol_SeparatorHovered] = accentDim;
    c[ImGuiCol_SeparatorActive] = accent;
    c[ImGuiCol_ResizeGrip] = bgHover;
    c[ImGuiCol_ResizeGripHovered] = accentDim;
    c[ImGuiCol_ResizeGripActive] = accent;
    c[ImGuiCol_TextSelectedBg] = ImVec4(accent.x, accent.y, accent.z, 0.40f);
    c[ImGuiCol_NavHighlight] = accent;
    c[ImGuiCol_Tab] = bgMid;
    c[ImGuiCol_TabHovered] = bgHover;
    c[ImGuiCol_TabActive] = accentDim;
    c[ImGuiCol_TabUnfocused] = bgMid;
    c[ImGuiCol_TabUnfocusedActive] = bgLight;
}

// ============================================================
// ХЕЛПЕРЫ
// ============================================================
static ImU32 ColU32(const ImVec4& c, float a = -1.0f) {
    ImVec4 v = c;
    if (a >= 0.0f) v.w = a;
    return ImGui::ColorConvertFloat4ToU32(v);
}

static float Pulse(float speed = 3.0f, float minV = 0.6f, float maxV = 1.0f) {
    float t = (float)ImGui::GetTime() * speed;
    return minV + (maxV - minV) * (0.5f + 0.5f * std::sin(t));
}

static void DrawGlowText(ImDrawList* dl, ImFont* font, float size,
    ImVec2 pos, ImU32 col, ImU32 glowCol,
    const char* text, float glowRadius = 1.5f) {
    for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy)
            if (dx || dy)
                dl->AddText(font, size,
                    ImVec2(pos.x + dx * glowRadius, pos.y + dy * glowRadius),
                    glowCol, text);
    dl->AddText(font, size, pos, col, text);
}

// ============================================================
// Pill-КНОПКА
// ============================================================
static bool PillButton(const char* id, const char* label, ImVec2 size,
    ImVec4 colA, ImVec4 colB,
    bool pulse = false, bool enabled = true) {
    ImGui::PushID(id);

    ImVec2 pos = ImGui::GetCursorScreenPos();
    if (size.x == 0) size.x = ImGui::CalcTextSize(label).x + 44;
    if (size.y == 0) size.y = ImGui::GetFrameHeight() + 6;

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, size.y * 0.5f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

    ImGui::InvisibleButton("##btn", size);
    bool pressed = enabled && ImGui::IsItemClicked();
    bool hovered = enabled && ImGui::IsItemHovered();
    bool active = enabled && ImGui::IsItemActive();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(6);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    float r = size.y * 0.5f;
    float pulseV = pulse ? Pulse(4.5f, 0.75f, 1.0f) : 1.0f;

    ImVec4 a = colA, b = colB;
    if (!enabled) {
        a = ImVec4(a.x * 0.35f, a.y * 0.35f, a.z * 0.35f, 0.55f);
        b = ImVec4(b.x * 0.35f, b.y * 0.35f, b.z * 0.35f, 0.55f);
    }
    else {
        float mul = 1.0f;
        if (active)       mul = 1.30f;
        else if (hovered) mul = 1.15f;
        a.x = std::min(a.x * mul, 1.0f);
        a.y = std::min(a.y * mul, 1.0f);
        a.z = std::min(a.z * mul, 1.0f);
        b.x = std::min(b.x * mul, 1.0f);
        b.y = std::min(b.y * mul, 1.0f);
        b.z = std::min(b.z * mul, 1.0f);
    }

    ImVec2 pMin = pos;
    ImVec2 pMax = ImVec2(pos.x + size.x, pos.y + size.y);
    float cx = pMin.x + size.x * 0.5f;
    float cy = pMin.y + size.y * 0.5f;

    if (enabled && (hovered || pulse)) {
        float glowA = (hovered ? 0.50f : 0.30f) * pulseV;
        for (int k = 3; k >= 1; --k) {
            float off = (float)k * 2.0f;
            float alpha = glowA * (1.0f - (float)k / 4.0f);
            dl->AddRectFilled(
                ImVec2(pMin.x - off, pMin.y - off),
                ImVec2(pMax.x + off, pMax.y + off),
                ColU32(colA, alpha), r + off);
        }
    }

    int totalW = (int)std::ceil(size.x);
    for (int px = 0; px < totalW; ++px) {
        float x0 = pMin.x + (float)px;
        float x1 = x0 + 1.0f;
        if (x1 > pMax.x) x1 = pMax.x;

        float localCx = x0 + 0.5f;
        float dxLeft = localCx - pMin.x;
        float dxRight = pMax.x - localCx;
        float dx = std::min(dxLeft, dxRight);

        float dy = 0.0f;
        if (dx < r) {
            float t = r - dx;
            dy = r - std::sqrt(std::max(0.0f, r * r - t * t));
        }

        float yTop = pMin.y + dy;
        float yBottom = pMax.y - dy;

        float t = (float)px / (float)std::max(1, totalW - 1);
        ImVec4 col(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t,
            1.0f);

        dl->AddRectFilled(ImVec2(x0, yTop), ImVec2(x1, yBottom), ColU32(col));
    }

    if (enabled) {
        float glossH = size.y * 0.42f;
        for (int px = 0; px < totalW; ++px) {
            float x0 = pMin.x + (float)px;
            float x1 = x0 + 1.0f;
            if (x1 > pMax.x) x1 = pMax.x;

            float localCx = x0 + 0.5f;
            float dxLeft = localCx - pMin.x;
            float dxRight = pMax.x - localCx;
            float dx = std::min(dxLeft, dxRight);

            float dy = 0.0f;
            if (dx < r) {
                float t = r - dx;
                dy = r - std::sqrt(std::max(0.0f, r * r - t * t));
            }

            float yTop = pMin.y + dy + 2.0f;
            float yBot = pMin.y + dy + glossH;
            if (yBot > pMax.y - dy) yBot = pMax.y - dy;

            if (yTop < yBot)
                dl->AddRectFilled(ImVec2(x0, yTop), ImVec2(x1, yBot),
                    IM_COL32(255, 255, 255, 45));
        }
    }

    if (enabled) {
        ImVec4 borderV = b;
        borderV.x = std::min(borderV.x + 0.20f, 1.0f);
        borderV.y = std::min(borderV.y + 0.20f, 1.0f);
        borderV.z = std::min(borderV.z + 0.20f, 1.0f);
        float bw = hovered ? 2.0f : 1.2f;
        dl->AddRect(pMin, pMax, ColU32(borderV, hovered ? 1.0f : 0.7f), r, 0, bw);
    }

    ImVec2 ts = ImGui::CalcTextSize(label);
    ImVec2 tp(cx - ts.x * 0.5f, cy - ts.y * 0.5f);
    if (enabled) {
        dl->AddText(ImVec2(tp.x + 1.0f, tp.y + 1.0f), IM_COL32(0, 0, 0, 110), label);
        dl->AddText(tp, IM_COL32(255, 255, 255, 255), label);
    }
    else {
        dl->AddText(tp, IM_COL32(210, 210, 220, 130), label);
    }

    ImGui::PopID();
    return pressed;
}

// ============================================================
// Тема TextEditor — VS Dark+
// ============================================================
static void ApplyEditorPalette(TextEditor& editor) {
    TextEditor::Palette p;

    p[(int)TextEditor::PaletteIndex::Default] = IM_COL32(220, 220, 230, 255);
    p[(int)TextEditor::PaletteIndex::Keyword] = IM_COL32(86, 156, 214, 255);
    p[(int)TextEditor::PaletteIndex::Number] = IM_COL32(181, 206, 168, 255);
    p[(int)TextEditor::PaletteIndex::String] = IM_COL32(206, 145, 120, 255);
    p[(int)TextEditor::PaletteIndex::CharLiteral] = IM_COL32(206, 145, 120, 255);
    p[(int)TextEditor::PaletteIndex::Punctuation] = IM_COL32(220, 220, 230, 255);
    p[(int)TextEditor::PaletteIndex::Preprocessor] = IM_COL32(155, 155, 155, 255);
    p[(int)TextEditor::PaletteIndex::Identifier] = IM_COL32(156, 220, 254, 255);
    p[(int)TextEditor::PaletteIndex::KnownIdentifier] = IM_COL32(220, 220, 170, 255);
    p[(int)TextEditor::PaletteIndex::PreprocIdentifier] = IM_COL32(155, 155, 155, 255);
    p[(int)TextEditor::PaletteIndex::Comment] = IM_COL32(106, 153, 85, 255);
    p[(int)TextEditor::PaletteIndex::MultiLineComment] = IM_COL32(106, 153, 85, 255);
    p[(int)TextEditor::PaletteIndex::Background] = IM_COL32(14, 12, 25, 255);
    p[(int)TextEditor::PaletteIndex::Cursor] = IM_COL32(179, 136, 255, 255);
    p[(int)TextEditor::PaletteIndex::Selection] = IM_COL32(76, 56, 140, 160);
    p[(int)TextEditor::PaletteIndex::ErrorMarker] = IM_COL32(255, 51, 51, 255);
    p[(int)TextEditor::PaletteIndex::Breakpoint] = IM_COL32(156, 220, 254, 255);
    p[(int)TextEditor::PaletteIndex::LineNumber] = IM_COL32(90, 90, 110, 255);
    p[(int)TextEditor::PaletteIndex::CurrentLineFill] = IM_COL32(24, 20, 38, 100);
    p[(int)TextEditor::PaletteIndex::CurrentLineFillInactive] = IM_COL32(24, 20, 38, 60);
    p[(int)TextEditor::PaletteIndex::CurrentLineEdge] = IM_COL32(139, 92, 246, 80);

    editor.SetPalette(p);
}

static TextEditor::LanguageDefinition MakeLangFor(int langIndex) {
    switch (langIndex) {
    case 0: return TextEditor::LanguageDefinition::CPlusPlus();
    case 1: return TextEditor::LanguageDefinition::C();
    case 2: return TextEditor::LanguageDefinition::CPlusPlus();
    case 3: return TextEditor::LanguageDefinition::CPlusPlus();
    case 4: return TextEditor::LanguageDefinition::CPlusPlus();
    case 5: return TextEditor::LanguageDefinition::CPlusPlus();
    }
    return TextEditor::LanguageDefinition::CPlusPlus();
}

// ============================================================
namespace deobf {
    namespace gui {

        AppWindow::AppWindow() {}
        AppWindow::~AppWindow() {
            if (m_worker.joinable()) m_worker.join();
        }

        void AppWindow::runDeobfuscatorAsync() {
            if (m_running) return;
            if (m_inputPath.empty()) {
                core::Logger::error("Не указан входной файл");
                return;
            }
            if (m_worker.joinable()) m_worker.join();

            m_running = true;
            m_hasResult = false;
            m_needsEditorUpdate = false;
            m_resultText.clear();

            m_worker = std::thread([this]() {
                try {
                    core::Config cfg;
                    cfg.inputPath = m_inputPath;
                    cfg.outputPath = m_inputPath + ".clean";
                    cfg.verbose = m_verbose;
                    cfg.dryRun = m_dryRun;

                    static const char* langs[] = { "cpp", "c", "csharp", "python", "js", "java" };
                    if (m_languageIndex < 0 || m_languageIndex > 5) m_languageIndex = 0;
                    cfg.language = langs[m_languageIndex];

                    std::vector<std::string> enabled = {
                        "define-expand",
                        "xor-strings",
                        "decoder",
                        "split-strings",
                        "char-math",
                        "const-fold",
                        "junk-ops",
                        "double-negation",
                        "opaque-predicates",
                        "identifiers",
                        "rename",
                        "dead-functions",
                        "empty-loops",
                        "collapse-lines",
                        "fake-branch",
                        "chr-chains",
                        "python-base64",
                        "python-base64-vars",
                        "python-xor-multi",
                        "python-xor",
                        "python-dead-funcs",
                        "reverse-strings",
                        "js-atob",
                        "js-charcode",
                        "js-reverse",
                        "js-xor",
                        "js-funcs",
                        // ★ Java — ПРАВИЛЬНЫЙ ПОРЯДОК:
                        //   1. Сначала сворачиваем base64/unicode/charcode/reverse ЛИТЕРАЛЫ
                        //   2. Потом const-propagate подставит значения в decode(var)
                        //   3. В следующем раунде base64 свернёт decode("base64-литерал")
                        "java-unicode-escape",
                        "java-base64",              // ← ПЕРЕД const-propagate
                        "java-charcode",
                        "java-reverse",
                        "java-string-xor",
                        "java-const-propagate",     // ← ПОСЛЕ base64
                        "java-dead-code",
                        "java-opaque",
                        "java-mba",
                        "java-identifiers",
                    };
                    cfg.enabledPasses = enabled;

                    core::Logger::info("Старт деобусификации...");
                    Deobfuscator d(cfg);
                    d.run();

                    try {
                        m_resultText = core::File::read(cfg.outputPath);
                        m_hasResult = true;
                        m_needsEditorUpdate = true;
                        core::Logger::info("Результат загружен (" +
                            std::to_string(countLines(m_resultText)) + " строк)");
                    }
                    catch (const std::exception& ex) {
                        core::Logger::error(std::string("Не могу прочитать результат: ") + ex.what());
                    }
                    core::Logger::info("Готово.");
                }
                catch (const std::exception& ex) {
                    core::Logger::error(std::string("Исключение: ") + ex.what());
                }
                catch (...) {
                    core::Logger::error("Неизвестное исключение");
                }
                m_running = false;
                });
        }

        void AppWindow::drawUI() {
            using namespace palette;

            ImGuiIO& io = ImGui::GetIO();
            ImDrawList* bgdl = ImGui::GetBackgroundDrawList();

            bgdl->AddRectFilledMultiColor(
                ImVec2(0, 0),
                ImVec2(io.DisplaySize.x, io.DisplaySize.y),
                ColU32(ImVec4(0.086f, 0.063f, 0.145f, 1.0f)),
                ColU32(ImVec4(0.086f, 0.063f, 0.145f, 1.0f)),
                ColU32(ImVec4(0.031f, 0.024f, 0.063f, 1.0f)),
                ColU32(ImVec4(0.031f, 0.024f, 0.063f, 1.0f)));

            {
                float t = (float)ImGui::GetTime();
                ImVec2 p1(io.DisplaySize.x * 0.15f + std::sin(t * 0.4f) * 40.0f,
                    io.DisplaySize.y * 0.20f + std::cos(t * 0.3f) * 30.0f);
                bgdl->AddCircleFilled(p1, 380.0f, ColU32(accent, 0.06f), 64);
                ImVec2 p2(io.DisplaySize.x * 0.85f + std::cos(t * 0.35f) * 50.0f,
                    io.DisplaySize.y * 0.75f + std::sin(t * 0.45f) * 40.0f);
                bgdl->AddCircleFilled(p2, 420.0f, ColU32(mint, 0.05f), 64);
            }

            float topH = 175.0f;
            float logH = 140.0f;
            float middleY = topH;
            float middleH = io.DisplaySize.y - topH - logH;

            // ============================================================
            // ВЕРХ
            // ============================================================
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, topH));
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 16));
            ImGui::Begin("##top", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                ImGuiWindowFlags_NoScrollbar);

            {
                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImVec2 p = ImGui::GetCursorScreenPos();
                ImFont* font = ImGui::GetFont();
                float size = ImGui::GetFontSize() * 1.6f;
                DrawGlowText(dl, font, size, p,
                    ColU32(textMain), ColU32(accent, 0.6f),
                    "Deobfuscator", 2.0f);
                ImGui::Dummy(ImVec2(0, size + 4));
                ImGui::SameLine();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);
                ImGui::TextColored(textDim, "v8.0");
                ImGui::SameLine();
                ImGui::TextColored(textMute, "\xC2\xB7");
                ImGui::SameLine();
                ImGui::TextColored(textDim, "37 pass-\xD0\xBE\xD0\xB2");
            }

            {
                const char* statusText = "\xD0\x9E\xD0\xB6\xD0\xB8\xD0\xB4\xD0\xB0\xD0\xBD\xD0\xB8\xD0\xB5";
                ImVec4 statusCol = textMute;
                if (m_running) {
                    statusText = "\xD0\xA0\xD0\xB0\xD0\xB1\xD0\xBE\xD1\x82\xD0\xB0\xD0\xB5\xD1\x82...";
                    statusCol = ImVec4(warn.x, warn.y, warn.z, Pulse(4.0f, 0.6f, 1.0f));
                }
                else if (m_hasResult) {
                    statusText = "\xD0\x93\xD0\xBE\xD1\x82\xD0\xBE\xD0\xB2\xD0\xBE";
                    statusCol = good;
                }
                else if (!m_inputPath.empty()) {
                    statusText = "\xD0\xA4\xD0\xB0\xD0\xB9\xD0\xBB \xD0\xB7\xD0\xB0\xD0\xB3\xD1\x80\xD1\x83\xD0\xB6\xD0\xB5\xD0\xBD";
                    statusCol = accentHot;
                }

                float textW = ImGui::CalcTextSize(statusText).x + 24;
                ImGui::SameLine(io.DisplaySize.x - textW - 20);
                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImVec2 p = ImGui::GetCursorScreenPos();
                float h = ImGui::GetTextLineHeight() + 6;

                ImVec2 pillMin(p.x - 6, p.y - 3);
                ImVec2 pillMax(p.x + textW - 6, p.y + h - 3);
                dl->AddRectFilled(pillMin, pillMax, ColU32(statusCol, 0.15f), 12.0f);
                dl->AddRect(pillMin, pillMax, ColU32(statusCol, 0.6f), 12.0f, 0, 1.0f);

                ImVec2 dot(p.x + 4, p.y + h * 0.5f - 3);
                dl->AddCircleFilled(dot, 4.0f, ColU32(statusCol));

                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 16);
                ImGui::TextColored(statusCol, "%s", statusText);
            }

            ImGui::Spacing();
            ImGui::Spacing();

            {
                char buf[512];
                strncpy_s(buf, m_inputPath.c_str(), sizeof(buf) - 1);

                ImGui::TextColored(textDim, "\xD0\xA4\xD0\xB0\xD0\xB9\xD0\xBB");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(-560);
                ImGui::PushStyleColor(ImGuiCol_FrameBg, bgCard);
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(accent.x, accent.y, accent.z, 0.3f));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 14.0f);
                if (ImGui::InputText("##in", buf, sizeof(buf))) m_inputPath = buf;
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);

                ImGui::SameLine();
                if (PillButton("browse",
                    "\xD0\x9E\xD0\xB1\xD0\xB7\xD0\xBE\xD1\x80",
                    ImVec2(110, 0),
                    cyan, accent)) {
                    std::string f = FileDialog::openFile(L"Выберите файл",
                        L"Все файлы\0*.*\0"
                        L"Java\0*.java\0"
                        L"C++\0*.cpp;*.h;*.hpp\0"
                        L"Python\0*.py\0"
                        L"JS\0*.js;*.ts\0");
                    if (!f.empty()) {
                        m_inputPath = f;
                        try {
                            m_sourceText = core::File::read(m_inputPath);
                            m_resultText.clear();
                            m_hasResult = false;
                            m_needsEditorUpdate = false;
                            core::Logger::info("Загружен: " + m_inputPath);
                        }
                        catch (const std::exception& ex) {
                            core::Logger::error(std::string("Не могу прочитать: ") + ex.what());
                        }
                    }
                }

                ImGui::SameLine();
                bool canRun = !m_running && !m_inputPath.empty();
                const char* runLabel = m_running
                    ? "\xD0\xA0\xD0\xB0\xD0\xB1\xD0\xBE\xD1\x82\xD0\xB0..."
                    : "\xE2\x96\xB6  \xD0\x97\xD0\xB0\xD0\xBF\xD1\x83\xD1\x81\xD0\xBA";
                if (PillButton("run", runLabel, ImVec2(150, 0),
                    accent, mint, m_running, canRun)) {
                    LogBuffer::instance().clear();
                    runDeobfuscatorAsync();
                }

                ImGui::SameLine();
                bool canSave = m_hasResult;
                if (PillButton("save",
                    "\xD0\xA1\xD0\xBE\xD1\x85\xD1\x80\xD0\xB0\xD0\xBD\xD0\xB8\xD1\x82\xD1\x8C",
                    ImVec2(150, 0),
                    mint, cyan, false, canSave)) {
                    std::string f = FileDialog::saveFile(L"Сохранить результат");
                    if (!f.empty()) {
                        try {
                            core::File::write(f, m_resultText);
                            core::Logger::info("Сохранено: " + f);
                        }
                        catch (const std::exception& ex) {
                            core::Logger::error(std::string("Не могу сохранить: ") + ex.what());
                        }
                    }
                }
            }

            ImGui::Spacing();

            {
                static const char* langs[] = { "cpp", "c", "csharp", "python", "js", "java" };
                ImGui::TextColored(textDim, "\xD0\xAF\xD0\xB7\xD1\x8B\xD0\xBA");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(140);
                ImGui::PushStyleColor(ImGuiCol_FrameBg, bgCard);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 14.0f);
                ImGui::Combo("##lang", &m_languageIndex, langs, IM_ARRAYSIZE(langs));
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();

                ImGui::SameLine(260);
                ImGui::PushStyleColor(ImGuiCol_CheckMark, accentHot);
                ImGui::Checkbox("\xD0\x9F\xD0\xBE\xD0\xB4\xD1\x80\xD0\xBE\xD0\xB1\xD0\xBD\xD1\x8B\xD0\xB9 \xD0\xBB\xD0\xBE\xD0\xB3", &m_verbose);
                ImGui::SameLine();
                ImGui::Checkbox("Dry-run", &m_dryRun);
                ImGui::PopStyleColor();
            }

            ImGui::End();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();

            // ============================================================
            // ЦЕНТР
            // ============================================================
            ImGui::SetNextWindowPos(ImVec2(0, middleY));
            ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, middleH));
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 12));
            ImGui::Begin("##editors", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);

            ImGui::PushStyleColor(ImGuiCol_ChildBg, bgCard);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(mint.x, mint.y, mint.z, 0.35f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.5f);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 14.0f);
            ImGui::BeginChild("##right", ImVec2(0, 0), true,
                ImGuiWindowFlags_NoScrollbar);

            {
                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImVec2 p = ImGui::GetCursorScreenPos();
                dl->AddCircleFilled(ImVec2(p.x + 6, p.y + ImGui::GetTextLineHeight() * 0.5f),
                    5.0f, ColU32(mint));
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 18);
                ImGui::TextColored(mint,
                    "\xD0\xA0\xD0\xB5\xD0\xB7\xD1\x83\xD0\xBB\xD1\x8C\xD1\x82\xD0\xB0\xD1\x82 "
                    "\xD0\xB4\xD0\xB5\xD0\xBE\xD0\xB1\xD1\x83\xD1\x81\xD0\xB8\xD1\x84\xD0\xB8\xD0\xBA\xD0\xB0\xD1\x86\xD0\xB8\xD0\xB8");
                ImGui::SameLine();
                if (m_hasResult)
                    ImGui::TextColored(textMute, "\xC2\xB7 %d \xD1\x81\xD1\x82\xD1\x80\xD0\xBE\xD0\xBA",
                        countLines(m_resultText));
                else
                    ImGui::TextColored(textMute, "\xC2\xB7 \xD0\xBF\xD1\x83\xD1\x81\xD1\x82\xD0\xBE");

                if (m_hasResult && !m_resultText.empty()) {
                    float bw = 150.0f;
                    ImGui::SameLine(ImGui::GetWindowWidth() - bw - 16);
                    if (PillButton("copy",
                        "\xD0\x9A\xD0\xBE\xD0\xBF\xD0\xB8\xD1\x80\xD0\xBE\xD0\xB2\xD0\xB0\xD1\x82\xD1\x8C",
                        ImVec2(bw, 0),
                        accent, mint)) {
                        ImGui::SetClipboardText(m_resultText.c_str());
                        core::Logger::info("Скопировано в буфер обмена (" +
                            std::to_string(m_resultText.size()) + " байт)");
                    }
                }
            }

            ImGui::Separator();

            if (m_hasResult && !m_resultText.empty()) {
                static size_t lastHash = 0;
                static int lastLang = -1;
                size_t currentHash = std::hash<std::string>{}(m_resultText);
                if (currentHash != lastHash) {
                    m_editorRight.SetText(m_resultText);
                    lastHash = currentHash;
                }
                if (lastLang != m_languageIndex) {
                    m_editorRight.SetLanguageDefinition(MakeLangFor(m_languageIndex));
                    ApplyEditorPalette(m_editorRight);
                    lastLang = m_languageIndex;
                }

                ImVec2 avail = ImGui::GetContentRegionAvail();
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.055f, 0.047f, 0.098f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
                m_editorRight.Render("##result_editor", avail, false);
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
            }
            else if (m_running) {
                ImVec4 c = warn;
                c.w = Pulse(3.5f, 0.5f, 1.0f);
                ImGui::TextColored(c,
                    "\xE2\x8F\xB3  \xD0\x94\xD0\xB5\xD0\xBE\xD0\xB1\xD1\x83\xD1\x81\xD0\xB8\xD1\x84\xD0\xB8\xD0\xBA\xD0\xB0\xD1\x86\xD0\xB8\xD1\x8F "
                    "\xD0\xB2 \xD0\xBF\xD1\x80\xD0\xBE\xD1\x86\xD0\xB5\xD1\x81\xD1\x81\xD0\xB5...");
            }
            else {
                ImGui::TextColored(textMute,
                    "\xD0\x97\xD0\xB4\xD0\xB5\xD1\x81\xD1\x8C \xD0\xBF\xD0\xBE\xD1\x8F\xD0\xB2\xD0\xB8\xD1\x82\xD1\x81\xD1\x8F "
                    "\xD0\xBA\xD0\xBE\xD0\xB4 \xD0\xBF\xD0\xBE\xD1\x81\xD0\xBB\xD0\xB5 "
                    "\xD0\xBD\xD0\xB0\xD0\xB6\xD0\xB0\xD1\x82\xD0\xB8\xD1\x8F "
                    "\xC2\xAB\xD0\x97\xD0\xB0\xD0\xBF\xD1\x83\xD1\x81\xD0\xBA\xD0\xB8\xD1\x82\xD1\x8C\xC2\xBB");
            }
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);

            ImGui::End();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();

            // ============================================================
            // НИЗ — Лог
            // ============================================================
            ImGui::SetNextWindowPos(ImVec2(0, middleY + middleH));
            ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, logH));
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 10));
            ImGui::Begin("##log", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);

            ImGui::PushStyleColor(ImGuiCol_ChildBg, bgCard);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(accent.x, accent.y, accent.z, 0.35f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.5f);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 14.0f);
            ImGui::BeginChild("##logcard", ImVec2(0, 0), true);

            {
                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImVec2 p = ImGui::GetCursorScreenPos();
                dl->AddCircleFilled(ImVec2(p.x + 6, p.y + ImGui::GetTextLineHeight() * 0.5f),
                    5.0f, ColU32(warn));
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 18);
                ImGui::TextColored(warn, "\xD0\x9B\xD0\xBE\xD0\xB3");

                ImGui::SameLine();
                size_t logCount = LogBuffer::instance().snapshot().size();
                ImGui::TextColored(textMute, "\xC2\xB7 %d", (int)logCount);

                float bw = 130.0f;
                ImGui::SameLine(ImGui::GetWindowWidth() - bw - 16);
                if (PillButton("clr",
                    "\xD0\x9E\xD1\x87\xD0\xB8\xD1\x81\xD1\x82\xD0\xB8\xD1\x82\xD1\x8C",
                    ImVec2(bw, 0),
                    pink, orange)) {
                    LogBuffer::instance().clear();
                }
            }

            ImGui::Separator();

            ImGui::BeginChild("##logscroll", ImVec2(0, 0), false,
                ImGuiWindowFlags_HorizontalScrollbar);

            auto lines = LogBuffer::instance().snapshot();
            for (const auto& l : lines) {
                if (l.find("[ERROR]") != std::string::npos)
                    ImGui::TextColored(err, "\xE2\x9C\x96 %s", l.c_str());
                else if (l.find("[WARN]") != std::string::npos)
                    ImGui::TextColored(warn, "\xE2\x9A\xA0 %s", l.c_str());
                else if (l.find("[DEBUG]") != std::string::npos)
                    ImGui::TextColored(textMute, "\xC2\xB7 %s", l.c_str());
                else
                    ImGui::TextColored(textMain, "%s", l.c_str());
            }
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f)
                ImGui::SetScrollHereY(1.0f);

            ImGui::EndChild();
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);
            ImGui::End();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }

        // ============================================================
        int AppWindow::run() {
            WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L,
                               GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
                               L"deobf_gui", nullptr };
            ::RegisterClassExW(&wc);

            int screenW = GetSystemMetrics(SM_CXSCREEN);
            int screenH = GetSystemMetrics(SM_CYSCREEN);

            int winW = static_cast<int>(screenW * 0.90f);
            int winH = static_cast<int>(screenH * 0.90f);
            int winX = (screenW - winW) / 2;
            int winY = (screenH - winH) / 2;

            HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Deobfuscator",
                WS_OVERLAPPEDWINDOW, winX, winY, winW, winH,
                nullptr, nullptr, wc.hInstance, nullptr);

            if (!CreateDeviceD3D(hwnd)) {
                CleanupDeviceD3D();
                ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
                return 1;
            }

            ::ShowWindow(hwnd, SW_MAXIMIZE);
            ::UpdateWindow(hwnd);

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

            ImFontConfig fontCfg;
            fontCfg.OversampleH = 2;
            fontCfg.OversampleV = 2;
            fontCfg.PixelSnapH = true;

            static const ImWchar ranges[] = {
                0x0020, 0x00FF,
                0x0400, 0x04FF,
                0x2116, 0x2116,
                0x2010, 0x203A,
                0x2200, 0x22FF,
                0x25A0, 0x25FF,
                0x2600, 0x26FF,
                0,
            };

            io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 18.0f, &fontCfg, ranges);
            io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 15.0f, &fontCfg, ranges);

            ApplyCustomTheme();

            m_editorLeft.SetReadOnly(true);
            m_editorRight.SetReadOnly(true);

            ImGui_ImplWin32_Init(hwnd);
            ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

            bool done = false;
            while (!done) {
                MSG msg;
                while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);
                    if (msg.message == WM_QUIT) done = true;
                }
                if (done) break;

                ImGui_ImplDX11_NewFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();

                drawUI();

                ImGui::Render();
                const float clear[4] = { 0.031f, 0.024f, 0.063f, 1.0f };
                g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
                g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear);
                ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

                g_pSwapChain->Present(1, 0);
            }

            if (m_worker.joinable()) m_worker.join();

            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();

            CleanupDeviceD3D();
            ::DestroyWindow(hwnd);
            ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
            return 0;
        }

    } // namespace gui
} // namespace deobf