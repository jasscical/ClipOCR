#pragma once

#include <QObject>
#include <QAbstractNativeEventFilter>
#include <QKeySequence>

#ifdef Q_OS_WIN
#  include <windows.h>
#endif

// ClipOCR - clipboard OCR utility.
// Global hotkey registration.
// Windows uses RegisterHotKey + a native event filter.
// NOTE: Qt6 removed QObject::nativeEvent, so we use QAbstractNativeEventFilter
// (which survives across Qt5/Qt6) instead of overriding nativeEvent().
// macOS/Linux are TODO (see implementation).
class GlobalHotkey : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit GlobalHotkey(QObject* parent = nullptr);
    ~GlobalHotkey() override;

    // Register a single-key global hotkey from a QKeySequence.
    // Returns true on success. Only one hotkey is tracked at a time.
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
