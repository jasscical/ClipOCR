#pragma once

#include <QObject>

class QSystemTrayIcon;
class QAction;

// ClipOCR - 剪贴板 OCR 小工具。
// 系统托盘图标，带右键菜单（设置 / 退出）。
class TrayIcon : public QObject
{
    Q_OBJECT
public:
    explicit TrayIcon(QObject* parent = nullptr);
    ~TrayIcon() override;

    void show();
    void showNotification(const QString& str_title, const QString& str_message);
    void retranslate();

signals:
    void showSettingsRequested();
    void quitRequested();

private:
    QSystemTrayIcon* m_pTray = nullptr;
    QAction* m_pShowAct = nullptr;
    QAction* m_pQuitAct = nullptr;
};
