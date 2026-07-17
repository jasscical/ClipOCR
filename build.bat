@echo off
setlocal
:: ============================================================
:: ClipOCR build script  (Qt 6.7.2 + MinGW 11.2, release)
:: Usage: double-click from project root, or run from CMD.
:: ============================================================
:: Machine-specific paths live in build.local.bat (gitignored, NOT committed).
:: It may set QT_DIR / MINGW_DIR / CMAKE_DIR for your local toolchain.
:: If build.local.bat is absent, the defaults below are used.
if exist "%~dp0build.local.bat" call "%~dp0build.local.bat"

:: ---- Defaults (adjust or create build.local.bat) ----
if not defined QT_DIR    set "QT_DIR=C:\Qt\6.7.2\mingw_64"
if not defined MINGW_DIR set "MINGW_DIR=%QT_DIR%\..\Tools\MinGW1120_64\bin"
if not defined CMAKE_DIR set "CMAKE_DIR=%QT_DIR%\..\Tools\CMake_64\bin"

:: IMPORTANT: MinGW bin FIRST in PATH to avoid ABI mismatch with
:: any older MinGW (e.g. 8.1) that may also be on system PATH.
set "PATH=%MINGW_DIR%;%QT_DIR%\bin;%CMAKE_DIR%;%PATH%"

cd /d "%~dp0"
if not exist build mkdir build

echo [1/3] Configure (qt-cmake.bat sets up Qt6 toolchain)...
call qt-cmake.bat -S . -B build -G "MinGW Makefiles"
if errorlevel 1 goto :fail

echo [2/3] Build...
cmake --build build --config Release
if errorlevel 1 goto :fail

echo [3/3] Deploy Qt runtime DLLs next to exe...
windeployqt --release build\clipocr.exe

echo.
echo Done. Run: build\clipocr.exe
goto :end

:fail
echo.
echo *** BUILD FAILED *** Check errors above.

:end
echo.
echo Window stays open. Press any key to close (scroll up to review).
pause
