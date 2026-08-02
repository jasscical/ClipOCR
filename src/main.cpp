#include <QApplication>
#include <QClipboard>
#include <QTextStream>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTranslator>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <atomic>

#include "Config.h"
#include "OcrEngine.h"
#include "ClipboardMonitor.h"
#include "GlobalHotkey.h"
#include "Tray.h"
#include "MainWindow.h"
#include "SettingsDialog.h"
#include "HistoryManager.h"
#include "AutoStarter.h"

// 为指定语言代码安装（或移除）界面翻译器。
// "en" 直接使用源码中的原字符串（无需翻译器）。
static void applyLanguage(QApplication& app, QTranslator& translator, const QString& str_lang)
{
    app.removeTranslator(&translator);
    if (str_lang != QStringLiteral("en")) {
        const QString str_qm = QStringLiteral(":/translations/clipocr_%1.qm").arg(str_lang);
        if (translator.load(str_qm))
            app.installTranslator(&translator);
    }
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);
    QApplication::setApplicationName(QStringLiteral("ClipOCR"));
    QApplication::setOrganizationName(QStringLiteral("clipocr"));

    Config config;
    config.load();

    // 单实例：固定服务名。第二个实例连上后即通知主实例显示并自行退出。
    const QString kServerName = QStringLiteral("ClipOCR-SingleInstance");
    QLocalSocket probe;
    probe.connectToServer(kServerName);
    if (probe.waitForConnected(500)) {
        QTextStream(&probe) << QStringLiteral("show\n");
        probe.waitForBytesWritten(500);
        return 0;   // 已有实例在运行，本次启动仅负责唤起它
    }

    // 在创建任何窗口部件之前先加载界面语言。
    QTranslator translator;
    applyLanguage(app, translator, config.language());

    // 依据配置同步开机启动注册表项（配置为唯一真相源）。
    AutoStarter::setAutoStart(config.autoStart());

    OcrEngine engine;
    engine.setTesseractPath(config.tesseractPath());
    engine.setTessdataDir(config.tessdataDir());
    engine.setLanguage(config.ocrLanguage());
    engine.setUpscaleFactor(config.upscaleFactor());

    ClipboardMonitor monitor;
    GlobalHotkey hotkey;
    TrayIcon tray;
    HistoryManager history;
    MainWindow window(&config, &history);
    SettingsDialog* p_settings = nullptr;

    auto showSettings = [&]() {
        if (!p_settings) {
            p_settings = new SettingsDialog(&config);
            QObject::connect(p_settings, &SettingsDialog::settingsApplied, [&]() {
                engine.setTesseractPath(config.tesseractPath());
                engine.setTessdataDir(config.tessdataDir());
                engine.setLanguage(config.ocrLanguage());
                engine.setUpscaleFactor(config.upscaleFactor());
                hotkey.unregisterHotkey();
                hotkey.registerHotkey(config.hotkey());
                // 若对话框中改了语言则重新应用。
                config.load();
                applyLanguage(app, translator, config.language());
                tray.retranslate();
                // 同步开机启动注册表项（勾选/取消立即生效）。
                AutoStarter::setAutoStart(config.autoStart());
            });
        }
        p_settings->loadFromConfig();
        p_settings->show();
        p_settings->raise();
        p_settings->activateWindow();
    };

    // 热键按下 -> 抓取剪贴板图片。
    QObject::connect(&hotkey, &GlobalHotkey::activated,
                     &monitor, &ClipboardMonitor::grabFromClipboard);

    // 抓到图片 -> OCR（后台线程）-> 写回文本 + 历史记录。
    // OCR 通过 QtConcurrent 执行，GUI 线程永不阻塞（不会"无响应"）。
    struct OcrResult {
        bool bOk = false;
        QString strText;
        QString strErr;
    };
    std::atomic<bool> s_bBusy{false};

    QObject::connect(&monitor, &ClipboardMonitor::imageCaptured,
                     [&](const QImage img) {
                         if (s_bBusy.load()) {
                             OcrEngine::log(QStringLiteral("main: imageCaptured but busy, ignored"));
                             return;  // 任务运行中忽略重入触发
                         }
                         s_bBusy.store(true);
                         OcrEngine::log(QStringLiteral("main: imageCaptured, submitting OCR job"));

                         if (config.showPopup())
                             tray.showNotification(
                                 QCoreApplication::translate("main", "ClipOCR"),
                                 QCoreApplication::translate("main", "Recognizing..."));

                         auto* p_watcher = new QFutureWatcher<OcrResult>();
                         QObject::connect(p_watcher, &QFutureWatcher<OcrResult>::finished,
                             p_watcher, [p_watcher, img, &config, &engine, &history, &tray, &s_bBusy]() {
                                 const OcrResult res = p_watcher->result();
                                 p_watcher->deleteLater();
                                 s_bBusy.store(false);
                                 OcrEngine::log(QStringLiteral("main: OCR finished, ok=%1 chars=%2")
                                    .arg(res.bOk).arg(res.strText.size()));
                                 if (res.bOk) {
                                     QApplication::clipboard()->setText(res.strText);
                                     history.addRecord(img, res.strText);
                                     if (config.showPopup())
                                         tray.showNotification(
                                             QCoreApplication::translate("main", "OCR Done"),
                                             QCoreApplication::translate("main", "%1 characters copied to clipboard")
                                                 .arg(res.strText.size()));
                                 } else {
                                     OcrEngine::log(QStringLiteral("main: OCR FAILED: %1").arg(res.strErr));
                                     if (config.showPopup())
                                         tray.showNotification(
                                             QCoreApplication::translate("main", "OCR Failed"), res.strErr);
                                 }
                             });

                         p_watcher->setFuture(QtConcurrent::run([&engine, img]() -> OcrResult {
                             OcrEngine::log(QStringLiteral("worker: OCR job started"));
                             OcrResult r;
                             r.bOk = engine.recognize(img, r.strText, r.strErr);
                             OcrEngine::log(QStringLiteral("worker: OCR job done, ok=%1").arg(r.bOk));
                             return r;
                         }));
                     });

    QObject::connect(&monitor, &ClipboardMonitor::noImageInClipboard,
                     [&]() {
                         if (config.showPopup())
                             tray.showNotification(
                                 QCoreApplication::translate("main", "ClipOCR"),
                                 QCoreApplication::translate("main", "No image found in clipboard"));
                     });

    // 主窗口请求。
    QObject::connect(&window, &MainWindow::settingsRequested, showSettings);
    QObject::connect(&window, &MainWindow::clearHistoryRequested, &history, &HistoryManager::clear);
    QObject::connect(&window, &MainWindow::languageChanged,
                     [&](const QString& str_lang) {
                         config.setLanguage(str_lang);
                         config.save();
                         applyLanguage(app, translator, str_lang);
                         tray.retranslate();
                     });

    QObject::connect(&tray, &TrayIcon::showSettingsRequested, showSettings);
    QObject::connect(&tray, &TrayIcon::quitRequested, &app, &QApplication::quit);

    hotkey.registerHotkey(config.hotkey());
    window.show();
    tray.show();

    // 作为主实例监听本地服务；后续实例连入时唤起本窗口（单实例跳转）。
    QLocalServer localServer;
    QLocalServer::removeServer(kServerName);   // 清理可能残留的 socket
    if (localServer.listen(kServerName)) {
        QObject::connect(&localServer, &QLocalServer::newConnection, [&]() {
            if (QLocalSocket* p_client = localServer.nextPendingConnection())
                p_client->deleteLater();   // 读到的 "show" 指令无需处理，仅用于唤起
            window.showNormal();
            window.raise();
            window.activateWindow();
        });
    }

    return app.exec();
}
