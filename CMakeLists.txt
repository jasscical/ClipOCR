cmake_minimum_required(VERSION 3.16)

project(ClipOCR
    VERSION 0.1.0
    DESCRIPTION "Clipboard image OCR to clipboard - cross-platform Qt6 tray tool"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Concurrent)
qt_standard_project_setup()

add_executable(clipocr
    src/main.cpp
    src/Config.h src/Config.cpp
    src/OcrEngine.h src/OcrEngine.cpp
    src/ImageProcessor.h src/ImageProcessor.cpp
    src/ClipboardMonitor.h src/ClipboardMonitor.cpp
    src/GlobalHotkey.h src/GlobalHotkey.cpp
    src/Tray.h src/Tray.cpp
    src/MainWindow.h src/MainWindow.cpp
    src/SettingsDialog.h src/SettingsDialog.cpp
    src/HistoryManager.h src/HistoryManager.cpp
    translations/translations.qrc
)

target_link_libraries(clipocr PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Concurrent
)

# Windows GUI 子系统：无控制台窗口。
if(WIN32)
    set_target_properties(clipocr PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# Tesseract OCR 直接通过 QProcess 调用（无需辅助脚本）。

# 构建时从 .ts 重新生成 .qm。若 PATH 上找不到 lrelease，则退回已提交的 .qm
# 文件，确保构建不会失败。
find_program(LRELEASE_EXECUTABLE lrelease)
if(LRELEASE_EXECUTABLE)
    set(LRELEASE_TS
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/clipocr_en.ts
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/clipocr_zh_CN.ts)
    set(LRELEASE_QM
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/clipocr_en.qm
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/clipocr_zh_CN.qm)
    add_custom_command(
        OUTPUT ${LRELEASE_QM}
        COMMAND ${LRELEASE_EXECUTABLE} ${LRELEASE_TS}
        DEPENDS ${LRELEASE_TS}
        COMMENT "Generating translation .qm files"
        VERBATIM)
    add_custom_target(update_translations DEPENDS ${LRELEASE_QM})
    add_dependencies(clipocr update_translations)
endif()
