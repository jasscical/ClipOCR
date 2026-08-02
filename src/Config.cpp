#include "Config.h"

#include <QCoreApplication>
#include <QFile>
#include <QSettings>
#include <QLocale>

const QString Config::ORGANIZATION = QStringLiteral("clipocr");
const QString Config::APP_NAME = QStringLiteral("ClipOCR");

Config::Config(QObject* parent) : QObject(parent)
{
    m_SeqHotkey = QKeySequence(QStringLiteral("Ctrl+Alt+O"));
    m_iUpscale = 3;
    m_bShowPopup = true;
    m_strTessPath = QStringLiteral("tesseract");
    m_strLanguage = defaultLanguage();
    m_strOcrLanguage = defaultOcrLanguage();
}

QString Config::configFilePath()
{
    return QCoreApplication::applicationDirPath() + QStringLiteral("/config.ini");
}

QString Config::defaultLanguage()
{
    return (QLocale().language() == QLocale::Chinese)
               ? QStringLiteral("zh_CN")
               : QStringLiteral("en");
}

// 图片文字的默认 OCR 语言：中文环境下为中文+英文，其他环境为纯英文。
// 用户可在设置中修改。
QString Config::defaultOcrLanguage()
{
    return (QLocale().language() == QLocale::Chinese)
               ? QStringLiteral("chi_sim+eng")
               : QStringLiteral("eng");
}

QString Config::ocrLanguage() const
{
    return m_strOcrLanguage;
}

void Config::setOcrLanguage(const QString& str_lang)
{
    m_strOcrLanguage = str_lang.trimmed();
}

QKeySequence Config::hotkey() const { return m_SeqHotkey; }
void Config::setHotkey(const QKeySequence& seq) { m_SeqHotkey = seq; }

int Config::upscaleFactor() const { return m_iUpscale; }
void Config::setUpscaleFactor(int i_factor) { m_iUpscale = qBound(1, i_factor, 4); }

bool Config::showPopup() const { return m_bShowPopup; }
void Config::setShowPopup(bool b_show) { m_bShowPopup = b_show; }

bool Config::autoStart() const { return m_bAutoStart; }
void Config::setAutoStart(bool b_auto) { m_bAutoStart = b_auto; }

QString Config::tesseractPath() const { return m_strTessPath; }
void Config::setTesseractPath(const QString& str_path)
{
    m_strTessPath = str_path.trimmed();
    if (m_strTessPath.isEmpty())
        m_strTessPath = QStringLiteral("tesseract");
}

QString Config::tessdataDir() const { return m_strTessdataDir; }
void Config::setTessdataDir(const QString& str_dir) { m_strTessdataDir = str_dir; }

QString Config::language() const { return m_strLanguage; }
void Config::setLanguage(const QString& str_lang) { m_strLanguage = str_lang; }

bool Config::firstRun() const
{
    return !QFile::exists(configFilePath());
}

void Config::load()
{
    // 配置文件从 clipocr.ini 改名为 config.ini 后，首次启动做一次迁移：
    // 若新文件不存在但旧的 clipocr.ini 在，则从旧文件读，并在 save() 时写回新位置。
    QString strPath = configFilePath();
    if (!QFile::exists(strPath)) {
        const QString strOld = QCoreApplication::applicationDirPath()
                               + QStringLiteral("/clipocr.ini");
        if (QFile::exists(strOld))
            strPath = strOld;
    }

    QSettings s(strPath, QSettings::IniFormat);
    m_SeqHotkey = QKeySequence(s.value(QStringLiteral("hotkey"), m_SeqHotkey.toString()).toString());
    m_iUpscale = s.value(QStringLiteral("upscale"), m_iUpscale).toInt();
    m_bShowPopup = s.value(QStringLiteral("showPopup"), m_bShowPopup).toBool();
    m_bAutoStart = s.value(QStringLiteral("auto_start"), m_bAutoStart).toBool();
    m_strTessPath = s.value(QStringLiteral("tesseractPath"), m_strTessPath).toString();
    m_strTessdataDir = s.value(QStringLiteral("tessdataDir"), m_strTessdataDir).toString();
    m_strOcrLanguage = s.value(QStringLiteral("ocr_language"), defaultOcrLanguage()).toString();
    const QString strLang = s.value(QStringLiteral("language"), defaultLanguage()).toString();
    // "language" 键保存的是界面语言。旧版配置曾把 OCR 语言存到这里
    // （如 "chi_sim+eng"）；遇到这种情况回落到界面语言默认值。
    m_strLanguage = (strLang == QStringLiteral("en") || strLang == QStringLiteral("zh_CN"))
                        ? strLang
                        : defaultLanguage();

    // 若本次是从旧文件迁移读取，则写回新位置并删除旧文件，完成迁移。
    if (strPath != configFilePath()) {
        save();
        QFile::remove(strPath);
    }
}

void Config::save()
{
    QSettings s(configFilePath(), QSettings::IniFormat);
    s.setValue(QStringLiteral("hotkey"), m_SeqHotkey.toString());
    s.setValue(QStringLiteral("upscale"), m_iUpscale);
    s.setValue(QStringLiteral("showPopup"), m_bShowPopup);
    s.setValue(QStringLiteral("auto_start"), m_bAutoStart);
    s.setValue(QStringLiteral("tesseractPath"), m_strTessPath);
    s.setValue(QStringLiteral("tessdataDir"), m_strTessdataDir);
    s.setValue(QStringLiteral("ocr_language"), m_strOcrLanguage);
    s.setValue(QStringLiteral("language"), m_strLanguage);
}
