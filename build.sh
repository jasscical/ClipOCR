#!/usr/bin/env bash
# ============================================================
# ClipOCR build script (Linux + MinGW-w64 cross-compile to Windows exe)
# Usage: ./build.sh   (from project root)
# ============================================================
# Machine-specific overrides live in build.local.sh (gitignored, NOT committed).
# It may set PREFIX / CMAKE / WINDEPLOYQT to your local MinGW-w64 toolchain.
if [[ -f "${BASH_SOURCE%/*}/build.local.sh" ]]; then
    source "${BASH_SOURCE%/*}/build.local.sh"
fi

# ---- Defaults (adjust or create build.local.sh) ----
# Most distros ship a `x86_64-w64-mingw32-cmake` wrapper that injects the
# cross toolchain automatically. Fall back to plain cmake + a toolchain file.
PREFIX="${PREFIX:-x86_64-w64-mingw32}"
CMAKE="${CMAKE:-${PREFIX}-cmake}"
TOOLCHAIN_FILE="${TOOLCHAIN_FILE:-/usr/share/mingw-w64/toolchain-${PREFIX}.cmake}"
WINDEPLOYQT="${WINDEPLOYQT:-${PREFIX}-windeployqt}"
BUILD_TYPE="${BUILD_TYPE:-Release}"

set -e
cd "$(dirname "$0")"

if [[ ! -x "$(command -v "$CMAKE")" ]]; then
    echo "cmake wrapper '$CMAKE' not found; falling back to cmake with TOOLCHAIN_FILE."
    CMAKE=cmake
fi

echo "[1/3] Configure (MinGW-w64 -> Windows exe)..."
if [[ "$CMAKE" == "cmake" ]]; then
    "$CMAKE" -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" -S . -B build -G "MinGW Makefiles"
else
    "$CMAKE" -S . -B build -G "MinGW Makefiles"
fi

echo "[2/3] Build..."
"$CMAKE" --build build --config "$BUILD_TYPE"

echo "[3/3] Deploy Qt runtime DLLs next to exe..."
"$WINDEPLOYQT" --release "build/clipocr.exe"

echo
echo "Done. Output: build/clipocr.exe (run on Windows)"
