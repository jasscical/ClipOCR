# ClipOCR

A lightweight, cross-platform **tray tool** that recognizes text from an image
currently sitting in your clipboard and writes the recognized text back to the
clipboard.

> **English** | **[中文](#中文)**

---

## English

A lightweight, cross-platform **tray tool** that recognizes text from an image
currently sitting in your clipboard and writes the recognized text back to the
clipboard.

> **Design philosophy:** we do **not** take a new screenshot. Many people already
> use a screenshot tool they love (Snipaste, WeChat, QQ, browser devtools, ...).
> ClipOCR watches the **clipboard**: you screenshot as usual -> the image lands in
> the clipboard -> press a hotkey -> text is recognized and copied back. It coexists
> with your existing tools instead of replacing them.

This is the C++/Qt6 successor of an earlier Windows-only PowerShell + AutoHotkey
prototype. We evaluated WinRT.Ocr (PowerShell compatibility issues), PP-OCRv4/ONNX
(over-engineered for a clipboard helper), and settled on **Tesseract** for its
simplicity and reliability.

### Features

- System tray icon (left-click settings / right-click quit)
- Global hotkey to trigger OCR on the current clipboard image
- High-quality upscale preprocessing (configurable 1x-4x) to boost small-text accuracy
- **OCR via Tesseract CLI** (`QProcess` → `tesseract`, `--psm 6 -oem 1`)
  - Needs `chi_sim.traineddata` for Chinese (download from tesseract-ocr GitHub)
- CJK-aware post-processing (removes spaces inserted between CJK characters)
- Optional success popup notification
- Settings persisted with `QSettings`

### Requirements

- **Windows** (Tesseract is cross-platform but this tool uses Windows-specific APIs)
- **Qt 6** (tested with 6.7.2) with the `Core`, `Gui`, `Widgets` modules
- **Tesseract** OCR engine installed and reachable via `PATH` (or configure path in Settings)
  - For Chinese: download `chi_sim.traineddata` and place it under a `tessdata` directory,
    then point **Tessdata dir** at it in Settings (no trailing slash).

### Build (Windows, Qt 6.7.2 + MinGW 11.2)

Using the bundled `qt-cmake.bat` (it sets up the toolchain). A one-click `build.bat`
does the same three steps:

```bat
REM from the project root
REM Qt is the MinGW build, so force the MinGW Makefiles generator
REM (without -G, CMake may pick Visual Studio and produce an ABI mismatch).
REM Set QT_DIR to your Qt 6.7.2 MinGW install root (example: C:\Qt\6.7.2\mingw_64)
SET "QT_DIR=C:\Qt\6.7.2\mingw_64"
SET PATH=%QT_DIR%\..\Tools\CMake_64\bin;%QT_DIR%\..\Tools\MinGW1120_64\bin;%QT_DIR%\bin;%PATH%
qt-cmake.bat -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -S . -B build
cmake --build build
```

Alternatively open the folder in **Qt Creator** and build normally.

To produce a portable Windows package (Qt DLLs are copied next to the exe):

```bat
windeployqt --release build\clipocr.exe
```

> Built with MinGW 11.2 (the compiler Qt 6.7.2 itself was built with) to avoid ABI
> mismatches. Do **not** mix in an older MinGW from another toolchain.

### Usage

1. Install [Tesseract](https://github.com/tesseract-ocr/tesseract) and make sure `tesseract.exe` is on your **PATH** (or set the path in Settings).
   For Chinese: download [`chi_sim.traineddata`](https://github.com/tesseract-ocr/tessdata/blob/main/chi_sim.traineddata)
   and put it under a `tessdata/` directory, then point **Tessdata dir** at it.
2. Run `clipocr` (it hides in the system tray).
3. Screenshot as usual so the image is in your clipboard
   (e.g. Snipaste: make sure "Copy screenshot to clipboard" is enabled).
4. Press the hotkey (default **Ctrl+Alt+O**).
5. The recognized text is now in your clipboard -> `Ctrl+V` to paste.

Right-click the tray icon for **Settings** (hotkey, upscale, popup, Tesseract path,
tessdata dir, language) and **Quit**.

### Configuration

Settings are stored via `QSettings` (INI file `clipocr.ini` next to the exe).

| Setting | Default | Notes |
|---------|---------|-------|
| hotkey | `Ctrl+Alt+O` | Single key combination |
| upscale | `3` | 1-4; higher = better small-text accuracy, more memory/time |
| showPopup | `true` | Show a tray notification on success/failure |
| tesseractPath | `tesseract` | Falls back to PATH if not found |
| tessdataDir | *(empty)* | Use Tesseract default unless set (no trailing slash) |
| language | `chi_sim+eng` | Passed to `-l` |

### Architecture

```
main
 ├─ QSystemTrayIcon         tray (settings / quit)
 ├─ GlobalHotkey (Win)      RegisterHotKey -> activated()
 ├─ ClipboardMonitor        QClipboard::image() -> imageCaptured()
 ├─ OcrEngine               upscale -> QProcess(tesseract --psm 6 -oem 1) -> post-process -> clipboard
 ├─ ImageProcessor          QImage::scaled(factor, SmoothTransformation)
 └─ Config (QSettings)      hotkey / upscale / showPopup / tesseractPath / tessdataDir / language
```

The hardest cross-platform problem (reading an image from the clipboard) is
solved by Qt: `QClipboard::image()` works on Windows, macOS and Linux.

### Roadmap / TODO

- [ ] macOS global hotkey (CGEventTap)
- [ ] Linux global hotkey (X11 `XGrabKey` / Wayland portal)
- [ ] Optional "auto OCR on new clipboard image" mode
- [ ] Packaging scripts for macOS (`.app`) and Linux (AppImage/Flatpak)

### License

MIT (see LICENSE).

---

## 中文

一个轻量、跨平台的**系统托盘工具**：识别当前剪贴板中的图片文字，并把识别结果
写回剪贴板。

> **设计理念：** 我们**不**重新截图。很多人已经有自己顺手的截图工具
> （Snipaste、微信、QQ、浏览器开发者工具……）。ClipOCR 只**监听剪贴板**：
> 你照常截图 -> 图片进入剪贴板 -> 按下热键 -> 文字被识别并复制回来。
> 它与你现有的工具**共存**，而不是取代它们。

本项目是早期 Windows-only（PowerShell + AutoHotkey 原型）的 C++/Qt6 继任版本。我们评估过
WinRT.Ocr（PowerShell 兼容性问题）、PP-OCRv4/ONNX（对剪贴板助手过度设计），最终选择 **Tesseract**
——简单可靠、社区成熟。

### 功能特性

- 系统托盘图标（左键设置 / 右键退出）
- 全局热键，对当前剪贴板图片触发 OCR
- 高质量放大预处理（可配置 1x–4x），提升小字识别率
- **OCR 使用 Tesseract CLI**（`QProcess` → `tesseract --psm 6 -oem 1`）
  - 需要自行安装 Tesseract 并下载 `chi_sim.traineddata` 中文语言包
- 中文（CJK）感知后处理（去除汉字之间被插入的空格）
- 可选的识别成功弹窗通知
- 使用 `QSettings` 持久化设置

### 环境要求

- **Windows**（Tesseract 跨平台，但本工具使用了 Windows 特有的 API）
- **Qt 6**（使用 6.7.2 测试）并包含 `Core`、`Gui`、`Widgets` 模块
- **Tesseract** OCR 引擎已安装且可通过 `PATH` 调用（或在设置里配置路径）
  - 中文识别需 `chi_sim.traineddata`，下载后放入某个 `tessdata` 目录，
    然后在设置里把 **Tessdata dir** 指向它（末尾不要加斜杠）。

### 构建（Windows，Qt 6.7.2 + MinGW 11.2）

推荐使用自带 `qt-cmake.bat`（自动配置工具链）。一键脚本 `build.bat` 已包含下面三步：

```bat
REM 在项目根目录执行
REM Qt 为 MinGW 版本，必须显式指定 MinGW Makefiles 生成器
REM （不带 -G 时 CMake 可能误选 Visual Studio，导致 ABI 不兼容）。
REM Set QT_DIR to your Qt 6.7.2 MinGW install root (example: C:\Qt\6.7.2\mingw_64)
SET "QT_DIR=C:\Qt\6.7.2\mingw_64"
SET PATH=%QT_DIR%\..\Tools\CMake_64\bin;%QT_DIR%\..\Tools\MinGW1120_64\bin;%QT_DIR%\bin;%PATH%
qt-cmake.bat -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -S . -B build
cmake --build build
```

也可以直接用 **Qt Creator** 打开文件夹正常构建。

生成可移植的 Windows 包（Qt 的 DLL 会复制到 exe 同级）：

```bat
windeployqt --release build\clipocr.exe
```

> 使用 MinGW 11.2（即 Qt 6.7.2 自身构建时所用的编译器），以避免 ABI 不兼容。
> 切勿混入其他工具链的旧版 MinGW。

### 使用说明

1. 安装 [Tesseract](https://github.com/tesseract-ocr/tesseract) 并确保 `tesseract.exe`
   在 **PATH** 中（或在设置里手动指定路径）。
   中文识别需下载 [`chi_sim.traineddata`](https://github.com/tesseract-ocr/tessdata/blob/main/chi_sim.traineddata)，
   放入某个 `tessdata/` 目录，然后在设置里把 **Tessdata dir** 指向它。
2. 运行 `clipocr`（它会隐藏在系统托盘中）。
3. 照常截图，让图片进入剪贴板
   （例如 Snipaste：请确认已开启"复制截图到剪贴板"）。
4. 按下热键（默认 **Ctrl+Alt+O**）。
5. 识别出的文字已经回到剪贴板 -> 按 `Ctrl+V` 粘贴即可。

右键托盘图标可进行**设置**（热键、放大倍数、弹窗开关、Tesseract 路径、
tessdata 目录、识别语言）和**退出**。

### 配置项

设置通过 `QSettings` 存储（INI 文件 `clipocr.ini` 在 exe 同级目录）。

| 配置项 | 默认值 | 说明 |
|--------|--------|------|
| hotkey | `Ctrl+Alt+O` | 单个按键组合 |
| upscale | `3` | 1–4；越大小字识别率越高，但更耗内存/时间 |
| showPopup | `true` | 成功/失败时显示托盘通知 |
| tesseractPath | `tesseract` | 留空则回退到 PATH 查找 |
| tessdataDir | *（空）* | 不填则用 Tesseract 默认目录（末尾不要加斜杠） |
| language | `chi_sim+eng` | 传给 `-l` 的语言参数 |

### 架构

```
main
 ├─ QSystemTrayIcon         托盘（设置 / 退出）
 ├─ GlobalHotkey (Win)      RegisterHotKey -> activated()
 ├─ ClipboardMonitor        QClipboard::image() -> imageCaptured()
 ├─ OcrEngine               放大 -> QProcess(tesseract) -> 后处理 -> 剪贴板
 ├─ ImageProcessor          QImage::scaled(factor, SmoothTransformation)
 └─ Config (QSettings)      热键 / 放大倍数 / 弹窗 / Tesseract路径 / tessdata目录 / 语言
```

最难的平台兼容问题（从剪贴板读取图片）由 Qt 解决：
`QClipboard::image()` 在 Windows、macOS 和 Linux 上均可工作。

### 路线图 / 待办

- [ ] macOS 全局热键（CGEventTap）
- [ ] Linux 全局热键（X11 `XGrabKey` / Wayland portal）
- [ ] 可选的"剪贴板新图片自动 OCR"模式
- [ ] macOS（`.app`）与 Linux（AppImage/Flatpak）打包脚本

### 许可证

MIT（见 LICENSE 文件）。
