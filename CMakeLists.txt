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

# Windows GUI subsystem: no console window.
if(WIN32)
    set_target_properties(clipocr PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# Tesseract OCR is invoked directly via QProcess (no helper script needed).

# Regenerate .qm from .ts when the translations change.
find_program(LRELEASE_EXECUTABLE lrelease)
if(LRELEASE_EXECUTABLE)
    foreach(_ts clipocr_en.ts clipocr_zh_CN.ts)
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_ts}.qm
            COMMAND ${LRELEASE_EXECUTABLE}
                    ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_ts}
                    -qm ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_ts}.qm
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_ts}
            VERBATIM)
    endforeach()
endif()
