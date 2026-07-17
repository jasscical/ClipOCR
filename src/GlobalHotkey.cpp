#include "GlobalHotkey.h"

#include <QCoreApplication>

GlobalHotkey::GlobalHotkey(QObject* parent) : QObject(parent)
{
    if (QCoreApplication::instance())
        QCoreApplication::instance()->installNativeEventFilter(this);
}

GlobalHotkey::~GlobalHotkey()
{
    if (QCoreApplication::instance())
        QCoreApplication::instance()->removeNativeEventFilter(this);
    unregisterHotkey();
}

bool GlobalHotkey::registerHotkey(const QKeySequence& seq)
{
    unregisterHotkey();
    m_SeqHotkey = seq;

#ifdef Q_OS_WIN
    m_uiNativeMods = 0;
    m_uiNativeKey = 0;

    if (seq.isEmpty())
        return false;

    const QKeyCombination kc = seq[0]; // first key combination
    const Qt::KeyboardModifiers mods = kc.keyboardModifiers();
    const int i_key = kc.key();
    if (mods & Qt::CTRL)  m_uiNativeMods |= MOD_CONTROL;
    if (mods & Qt::ALT)   m_uiNativeMods |= MOD_ALT;
    if (mods & Qt::SHIFT) m_uiNativeMods |= MOD_SHIFT;
    if (mods & Qt::META)  m_uiNativeMods |= MOD_WIN;

    // Qt key codes for letters/digits/F-keys match Win32 VK_* codes.
    m_uiNativeKey = static_cast<quint32>(i_key) & 0xFFFF;

    if (!RegisterHotKey(nullptr, HOTKEY_ID, m_uiNativeMods, m_uiNativeKey)) {
        return false;
    }
    m_bRegistered = true;
    return true;
#else
    Q_UNUSED(seq);
    // TODO(cross-platform): macOS -> CGEventTap; Linux/X11 -> XGrabKey; Wayland -> portal
    return false;
#endif
}

void GlobalHotkey::unregisterHotkey()
{
#ifdef Q_OS_WIN
    if (m_bRegistered) {
        UnregisterHotKey(nullptr, HOTKEY_ID);
        m_bRegistered = false;
    }
#else
    // nothing to unregister yet
#endif
}

#ifdef Q_OS_WIN
bool GlobalHotkey::nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result)
{
    Q_UNUSED(eventType);
    MSG* p_msg = static_cast<MSG*>(message);
    if (p_msg && p_msg->message == WM_HOTKEY && p_msg->wParam == HOTKEY_ID) {
        emit activated();
        if (result)
            *result = 0;
        return true;
    }
    return false;
}
#endif
