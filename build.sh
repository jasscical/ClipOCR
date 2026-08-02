#!/usr/bin/env bash
# ============================================================
# ClipOCR 构建脚本
#   支持三种模式，自动探测：
#     1) Linux  : 交叉编译 Windows exe（x86_64-w64-mingw32-cmake wrapper）
#     2) Windows: 本机编译（自动定位 Qt 自带的 cmake + Qt6 工具链，MinGW）
#     3) 通用   : 使用 PATH 中的 cmake（需自行提供工具链，如原生 Linux/macOS 开发）
# 用法：./build.sh   （在项目根目录运行）
# ============================================================
# 机器相关覆盖写在 build.local.sh（已被 .gitignore，不提交）。
# 可设置：CMAKE / QT_TOOLCHAIN_FILE / QT_BASE / WINDEPLOYQT / MODE
if [[ -f "${BASH_SOURCE%/*}/build.local.sh" ]]; then
    source "${BASH_SOURCE%/*}/build.local.sh"
fi

set -euo pipefail
cd "$(dirname "$0")"

BUILD_DIR="${BUILD_DIR:-build}"
BUILD_TYPE="${BUILD_TYPE:-Release}"

# ---- 探测 Windows 上的 Qt 安装（绝对路径，避免相对 .. 解析问题）----
# 命中后设置：CMAKE(cmake.exe 完整路径) / QT_CMAKE_DIR / QT_BASE(mingw_64) / QT_MINGW_DIR
detect_windows_qt() {
    local r
    shopt -s nullglob
    for r in /c /e /f; do
        # cmake.exe
        for c in "$r"/ITSoftware/QT/qt6/Tools/CMake_*/bin/cmake.exe \
                 "$r"/ITSoftware/QT/Tools/CMake_*/bin/cmake.exe \
                 "$r"/Qt/*/Tools/CMake_*/bin/cmake.exe \
                 "$r"/Qt/Tools/CMake_*/bin/cmake.exe ; do
            QT_CMAKE_DIR="$(dirname "$c")"; CMAKE="$c"; break 2
        done
    done
    for r in /c /e /f; do
        # mingw_64 根（提供 Qt6 工具链与 windeployqt）
        for b in "$r"/ITSoftware/QT/qt6/*/mingw_64 \
                 "$r"/Qt/*/mingw_64 ; do
            QT_BASE="$b"; break 2
        done
    done
    for r in /c /e /f; do
        # Qt 自带 MinGW 编译器（提供 mingw32-make）
        for m in "$r"/ITSoftware/QT/qt6/Tools/MinGW*/bin/mingw32-make.exe \
                 "$r"/Qt/Tools/MinGW*/bin/mingw32-make.exe ; do
            QT_MINGW_DIR="$(dirname "$m")"; break 2
        done
    done
    shopt -u nullglob
}

# ---- 1) 定位 cmake 并确定模式 ----
if [[ -n "${CMAKE:-}" ]]; then
    # 已由 build.local.sh 设置：判断是否交叉编译 wrapper
    if [[ "$CMAKE" == *"x86_64-w64-mingw32-cmake"* ]]; then MODE="cross"; else MODE="generic"; fi
elif command -v x86_64-w64-mingw32-cmake >/dev/null 2>&1; then
    CMAKE="x86_64-w64-mingw32-cmake"; MODE="cross"
elif command -v cmake >/dev/null 2>&1; then
    CMAKE="cmake"; MODE="generic"
else
    detect_windows_qt
    [[ -n "${CMAKE:-}" ]] && MODE="windows"
fi

if [[ -z "${CMAKE:-}" ]]; then
    echo "错误：未找到 cmake。请安装 cmake，或在 build.local.sh 中设置 CMAKE 变量。" >&2
    exit 1
fi

# ---- 2) Windows 本机模式：注入 PATH ----
if [[ "${MODE:-}" == "windows" ]]; then
    if [[ -z "${QT_BASE:-}" || -z "${QT_CMAKE_DIR:-}" ]]; then
        echo "Windows 模式：未能自动定位 Qt 安装，请在 build.local.sh 设置 QT_BASE / CMAKE。" >&2
        exit 1
    fi
    QT_TOOLCHAIN_FILE="${QT_TOOLCHAIN_FILE:-$QT_BASE/lib/cmake/Qt6/qt.toolchain.cmake}"
    QT_MINGW_DIR="${QT_MINGW_DIR:-}"
    # 把 Qt 的 MinGW bin 放在 PATH 最前，避免旧版本 MinGW 抢占导致 ABI 不兼容
    export PATH="${QT_MINGW_DIR:+$QT_MINGW_DIR:}$QT_BASE/bin:$QT_CMAKE_DIR:$PATH"
    CMAKE=cmake
    WINDEPLOYQT=windeployqt
fi

# ---- 3) 组装配置参数 ----
CFG=(-S . -B "$BUILD_DIR" -G "MinGW Makefiles")
if [[ "${MODE:-}" == "cross" ]]; then
    : # 交叉编译 wrapper 已自动注入工具链
elif [[ -n "${QT_TOOLCHAIN_FILE:-}" ]]; then
    CFG+=("-DCMAKE_TOOLCHAIN_FILE=$QT_TOOLCHAIN_FILE")
fi
# 通用模式且无 QT_TOOLCHAIN_FILE 时不指定工具链，交由 cmake 自动查找 Qt6

echo "[1/3] 配置 (mode=${MODE:-generic}, cmake=$CMAKE)..."
"$CMAKE" "${CFG[@]}"

echo "[2/3] 构建..."
"$CMAKE" --build "$BUILD_DIR" --config "$BUILD_TYPE"

echo "[3/3] 部署 Qt 运行时依赖..."
if [[ "${MODE:-}" == "cross" ]]; then
    WINDEPLOYQT="${WINDEPLOYQT:-x86_64-w64-mingw32-windeployqt}"
    "$WINDEPLOYQT" --release "$BUILD_DIR/clipocr.exe"
elif [[ "${MODE:-}" == "windows" ]]; then
    windeployqt --release "$BUILD_DIR/clipocr.exe"
fi

echo
echo "完成。产物：$BUILD_DIR/clipocr.exe"
