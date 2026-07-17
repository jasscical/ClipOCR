<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN">
<context>
    <name>MainWindow</name>
    <message><source>ClipOCR</source><translation>ClipOCR</translation></message>
    <message><source>Settings</source><translation>设置</translation></message>
    <message><source>Clear History</source><translation>清空历史</translation></message>
    <message><source>About</source><translation>关于</translation></message>
    <message><source>Language:</source><translation>语言：</translation></message>
    <message><source>English</source><translation>English</translation></message>
    <message><source>Chinese</source><translation>中文</translation></message>
    <message><source>History</source><translation>历史记录</translation></message>
    <message><source>Copy Text</source><translation>复制文字</translation></message>
    <message><source>Copy Image</source><translation>复制图片</translation></message>
    <message><source>No history yet. Capture an image to the clipboard and press the hotkey to begin.</source><translation>暂无历史。将图片截取到剪贴板后按快捷键即可开始。</translation></message>
    <message><source>Time:</source><translation>时间：</translation></message>
    <message><source>Copied to clipboard</source><translation>已复制到剪贴板</translation></message>
    <message><source>ClipOCR - clipboard image to text.

Capture an image to the clipboard (e.g. Snipaste), then press the hotkey to OCR it and copy the text.</source><translation>ClipOCR - 剪贴板图片转文字。

将图片截取到剪贴板（如 Snipaste），然后按快捷键进行 OCR 并复制文字。</translation></message>
</context>
<context>
    <name>SettingsDialog</name>
    <message><source>Settings</source><translation>设置</translation></message>
    <message><source>Browse...</source><translation>浏览...</translation></message>
    <message><source>Save</source><translation>保存</translation></message>
    <message><source>Reset to defaults</source><translation>恢复默认</translation></message>
    <message><source>Usage: screenshot to clipboard (e.g. Snipaste), then press the hotkey.
Requires Tesseract OCR engine installed. Download chi_sim.traineddata for
Chinese and place it in the tessdata directory.</source><translation>用法：将图片截取到剪贴板（如 Snipaste），然后按快捷键。
需要安装 Tesseract OCR 引擎。识别中文请下载 chi_sim.traineddata 并
放置到 tessdata 目录。</translation></message>
    <message><source>Hotkey</source><translation>快捷键</translation></message>
    <message><source>Upscale factor (1-4)</source><translation>放大倍数 (1-4)</translation></message>
    <message><source>Show popup on success</source><translation>识别成功时弹窗提示</translation></message>
    <message><source>OCR recognition language</source><translation>OCR 识别语言</translation></message>
    <message><source>Language of the text in the screenshots (Tesseract code, e.g. chi_sim+eng).</source><translation>截图里文字的语种（Tesseract 代码，如 chi_sim+eng）。</translation></message>
    <message><source>Chinese + English</source><translation>中文 + 英文</translation></message>
    <message><source>English</source><translation>英文</translation></message>
    <message><source>Chinese (simplified)</source><translation>中文（简体）</translation></message>
    <message><source>Chinese (traditional)</source><translation>中文（繁体）</translation></message>
    <message><source>Japanese</source><translation>日语</translation></message>
    <message><source>Korean</source><translation>韩语</translation></message>
    <message><source>French</source><translation>法语</translation></message>
    <message><source>German</source><translation>德语</translation></message>
    <message><source>Spanish</source><translation>西班牙语</translation></message>
    <message><source>Russian</source><translation>俄语</translation></message>
    <message><source>Tesseract path</source><translation>Tesseract 路径</translation></message>
    <message><source>Tessdata directory</source><translation>语言包目录</translation></message>
    <message><source>Select tessdata directory</source><translation>选择 tessdata 目录</translation></message>
</context>
<context>
    <name>TrayIcon</name>
    <message><source>Settings...</source><translation>设置...</translation></message>
    <message><source>Quit</source><translation>退出</translation></message>
    <message><source>ClipOCR - clipboard image to text</source><translation>ClipOCR - 剪贴板图片转文字</translation></message>
</context>
<context>
    <name>main</name>
    <message><source>ClipOCR</source><translation>ClipOCR</translation></message>
    <message><source>Recognizing...</source><translation>识别中...</translation></message>
    <message><source>OCR Done</source><translation>识别完成</translation></message>
    <message><source>%1 characters copied to clipboard</source><translation>%1 个字符已复制到剪贴板</translation></message>
    <message><source>OCR Failed</source><translation>识别失败</translation></message>
    <message><source>No image found in clipboard</source><translation>剪贴板中没有图片</translation></message>
</context>
<context>
    <name>OcrEngine</name>
    <message><source>Empty image</source><translation>空图片</translation></message>
    <message><source>Failed to create temporary file.</source><translation>无法创建临时文件。</translation></message>
    <message><source>Failed to write temporary image.</source><translation>无法写入临时图片。</translation></message>
    <message><source>Failed to create temporary output file.</source><translation>无法创建临时输出文件。</translation></message>
    <message><source>Failed to start Tesseract: </source><translation>无法启动 Tesseract：</translation></message>
    <message><source>
Make sure Tesseract is installed and on PATH, or set the correct path in Settings.</source><translation>
请确认 Tesseract 已安装且在 PATH 中，或在设置中填写正确路径。</translation></message>
    <message><source>OCR timed out (60s).</source><translation>OCR 超时（60 秒）。</translation></message>
    <message><source>OCR failed (exit code %1): </source><translation>OCR 失败（退出码 %1）：</translation></message>
    <message><source>
Check that the language data (e.g., chi_sim.traineddata) is available in the tessdata directory.</source><translation>
请确认语言包（如 chi_sim.traineddata）已置于 tessdata 目录中。</translation></message>
    <message><source>Failed to read OCR result.</source><translation>无法读取识别结果。</translation></message>
    <message><source>Missing language data: %1. Download the corresponding .traineddata into the tessdata directory.</source><translation>缺少语言包：%1。请将对应的 .traineddata 下载到 tessdata 目录中。</translation></message>
</context>
</TS>
