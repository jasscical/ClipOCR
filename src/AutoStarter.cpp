#include "AutoStarter.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

#if defined(Q_OS_WIN)
// 注册表 Run 项位置与本项目在注册表中的键名。
static const char* const kRunKey =
    "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const char* const kValueName = "ClipOCR";
#endif

void AutoStarter::setAutoStart(bool b_auto)
{
#if defined(Q_OS_WIN)
    QSettings reg(QString::fromLocal8Bit(kRunKey), QSettings::NativeFormat);
    if (b_auto) {
        // 使用原生分隔符的绝对路径，确保 Run 项能正确启动 exe。
        reg.setValue(QString::fromLocal8Bit(kValueName),
                     QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
    } else {
        reg.remove(QString::fromLocal8Bit(kValueName));
    }
#else
    // 非 Windows 平台暂不实现（交叉编译目标仍是 Windows，故不会被触发）。
    Q_UNUSED(b_auto)
#endif
}

bool AutoStarter::isAutoStart()
{
#if defined(Q_OS_WIN)
    QSettings reg(QString::fromLocal8Bit(kRunKey), QSettings::NativeFormat);
    return reg.contains(QString::fromLocal8Bit(kValueName));
#else
    return false;
#endif
}
