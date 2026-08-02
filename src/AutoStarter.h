#pragma once

// ClipOCR - 剪贴板 OCR 小工具。
// 开机启动：在 Windows 下写入/删除注册表 Run 项；非 Windows 平台为空实现。
class AutoStarter
{
public:
    // 开启或关闭开机启动。开启时把当前 exe 路径写入
    // HKCU\Software\Microsoft\Windows\CurrentVersion\Run 的 "ClipOCR" 项；
    // 关闭时删除该值。b_auto 为 false 时即便注册表残留也会被清理。
    static void setAutoStart(bool b_auto);

    // 当前是否已配置开机启动（仅 Windows 有意义，其他平台返回 false）。
    static bool isAutoStart();
};
