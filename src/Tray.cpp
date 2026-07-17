#include "Tray.h"

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QPainter>

TrayIcon::TrayIcon(QObject* parent) : QObject(parent)
{
    // Build a simple teal circle icon without external binary assets.
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(QStringLiteral("#2dd4bf")));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(4, 4, 24, 24);
    painter.end();

    m_pTray = new QSystemTrayIcon(QIcon(pixmap), this);

    auto* p_menu = new QMenu();
    m_pShowAct = p_menu->addAction(QStringLiteral("Settings..."));
    m_pQuitAct = p_menu->addAction(QStringLiteral("Quit"));

    QObject::connect(m_pShowAct, &QAction::triggered, this, &TrayIcon::showSettingsRequested);
    QObject::connect(m_pQuitAct, &QAction::triggered, this, &TrayIcon::quitRequested);

    m_pTray->setContextMenu(p_menu);
    m_pTray->setToolTip(QStringLiteral("ClipOCR - clipboard image to text"));
}

TrayIcon::~TrayIcon()
{
    if (m_pTray) {
        m_pTray->hide();
        delete m_pTray;
    }
}

void TrayIcon::show()
{
    if (m_pTray)
        m_pTray->show();
}

void TrayIcon::showNotification(const QString& str_title, const QString& str_message)
{
    if (m_pTray)
        m_pTray->showMessage(str_title, str_message, QSystemTrayIcon::Information, 3000);
}

void TrayIcon::retranslate()
{
    if (m_pShowAct) m_pShowAct->setText(tr("Settings..."));
    if (m_pQuitAct) m_pQuitAct->setText(tr("Quit"));
    if (m_pTray) m_pTray->setToolTip(tr("ClipOCR - clipboard image to text"));
}
