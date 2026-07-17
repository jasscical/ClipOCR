#include <QApplication>
#include <QClipboard>
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

// Install (or remove) the UI translator for the given language code.
// "en" uses the source strings (no translator needed).
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

    // Load UI language before any widget is created.
    QTranslator translator;
    applyLanguage(app, translator, config.language());

    OcrEngine engine;
    engine.setTesseractPath(config.tesseractPath());
    engine.setTessdataDir(config.tessdataDir());
    engine.setLanguage(config.language());
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
                engine.setLanguage(config.language());
                engine.setUpscaleFactor(config.upscaleFactor());
                hotkey.unregisterHotkey();
                hotkey.registerHotkey(config.hotkey());
                // Re-apply language in case it changed in the dialog.
                config.load();
                applyLanguage(app, translator, config.language());
                tray.retranslate();
            });
        }
        p_settings->loadFromConfig();
        p_settings->show();
        p_settings->raise();
        p_settings->activateWindow();
    };

    // Hotkey pressed -> grab clipboard image.
    QObject::connect(&hotkey, &GlobalHotkey::activated,
                     &monitor, &ClipboardMonitor::grabFromClipboard);

    // Image captured -> OCR (in a background thread) -> write text back + history.
    // OCR runs via QtConcurrent so the GUI thread never blocks (no "not responding").
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
                             return;  // ignore re-entrant triggers while a job is running
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

    // MainWindow requests.
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

    return app.exec();
}
