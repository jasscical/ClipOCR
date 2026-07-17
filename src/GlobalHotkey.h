#pragma once

#include <QObject>
#include <QAbstractNativeEventFilter>
#include <QKeySequence>

#ifdef Q_OS_WIN
#  include <windows.h>
#endif

// ClipOCR - 剪贴板 OCR 小工具。
// 全局热键注册。
// Windows 使用 RegisterHotKey + 原生事件过滤器。
// 注意：Qt6 移除了 QObject::nativeEvent，因此改用 QAbstractNativeEventFilter
// （Qt5/Qt6 通用），而非重写 nativeEvent()。
// macOS/Linux 待实现（见具体实现）。
class GlobalHotkey : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit GlobalHotkey(QObject* parent = nullptr);
    ~GlobalHotkey() override;

    // 从 QKeySequence 注册一个单键全局热键。
    // 成功返回 true。同一时刻只跟踪一个热键。
    bool registerHotkey(const QKeySequence& seq);
    void unregisterHotkey();

signals:
    void activated();

protected:
#ifdef Q_OS_WIN
    bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;
#endif

private:
    QKeySequence m_SeqHotkey;
    bool m_bRegistered = false;
#ifdef Q_OS_WIN
    quint32 m_uiNativeMods = 0;
    quint32 m_uiNativeKey = 0;
    static constexpr quint32 HOTKEY_ID = 1;
#endif
};
