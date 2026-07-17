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
}

QString Config::configFilePath()
{
    return QCoreApplication::applicationDirPath() + QStringLiteral("/clipocr.ini");
}

QString Config::defaultLanguage()
{
    return (QLocale().language() == QLocale::Chinese)
               ? QStringLiteral("chi_sim+eng")
               : QStringLiteral("eng");
}

QKeySequence Config::hotkey() const { return m_SeqHotkey; }
void Config::setHotkey(const QKeySequence& seq) { m_SeqHotkey = seq; }

int Config::upscaleFactor() const { return m_iUpscale; }
void Config::setUpscaleFactor(int i_factor) { m_iUpscale = qBound(1, i_factor, 4); }

bool Config::showPopup() const { return m_bShowPopup; }
void Config::setShowPopup(bool b_show) { m_bShowPopup = b_show; }

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
    QSettings s(configFilePath(), QSettings::IniFormat);
    m_SeqHotkey = QKeySequence(s.value(QStringLiteral("hotkey"), m_SeqHotkey.toString()).toString());
    m_iUpscale = s.value(QStringLiteral("upscale"), m_iUpscale).toInt();
    m_bShowPopup = s.value(QStringLiteral("showPopup"), m_bShowPopup).toBool();
    m_strTessPath = s.value(QStringLiteral("tesseractPath"), m_strTessPath).toString();
    m_strTessdataDir = s.value(QStringLiteral("tessdataDir"), m_strTessdataDir).toString();
    m_strLanguage = s.value(QStringLiteral("language"), m_strLanguage).toString();
}

void Config::save()
{
    QSettings s(configFilePath(), QSettings::IniFormat);
    s.setValue(QStringLiteral("hotkey"), m_SeqHotkey.toString());
    s.setValue(QStringLiteral("upscale"), m_iUpscale);
    s.setValue(QStringLiteral("showPopup"), m_bShowPopup);
    s.setValue(QStringLiteral("tesseractPath"), m_strTessPath);
    s.setValue(QStringLiteral("tessdataDir"), m_strTessdataDir);
    s.setValue(QStringLiteral("language"), m_strLanguage);
}
