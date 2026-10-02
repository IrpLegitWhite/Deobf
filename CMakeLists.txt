cmake_minimum_required(VERSION 3.15)
project(deobf CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# --- Наш код ---
file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# --- ImGui ---
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui")
set(IMGUI_SOURCES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_win32.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_dx11.cpp"
)

# --- ImGuiColorTextEdit ---
set(TEXTEDIT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ImGuiColorTextEdit")
set(TEXTEDIT_SOURCES
    "${TEXTEDIT_DIR}/TextEditor.cpp"
)

add_executable(deobf ${SOURCES} ${IMGUI_SOURCES} ${TEXTEDIT_SOURCES})

target_include_directories(deobf PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends"
    "${TEXTEDIT_DIR}"
)

if(MSVC)
    target_compile_options(deobf PRIVATE /W4 /wd5220 /wd4239)
    target_compile_definitions(deobf PRIVATE _CRT_SECURE_NO_WARNINGS)

    # ====== УБИРАЕМ КОНСОЛЬ ======
    # /SUBSYSTEM:WINDOWS — приложение GUI, без консольного окна
    # /ENTRY:mainCRTStartup — точка входа main(), но система не создаёт окно cmd
    target_link_options(deobf PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
else()
    target_compile_options(deobf PRIVATE -Wall -Wextra)
endif()

if(WIN32)
    target_link_libraries(deobf PRIVATE
        d3d11 dxgi dwmapi comctl32 msimg32
    )
endif()

set_target_properties(deobf PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)