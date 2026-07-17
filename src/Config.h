#pragma once

#include <QObject>
#include <QKeySequence>
#include <QString>

// ClipOCR - clipboard OCR utility.
// Persistent settings, stored in an INI file (clipocr.ini) next to the exe.
// See docs/CODING_STYLE.md.
class Config : public QObject
{
    Q_OBJECT
public:
    explicit Config(QObject* parent = nullptr);

    QKeySequence hotkey() const;
    void setHotkey(const QKeySequence& seq);

    int upscaleFactor() const;
    void setUpscaleFactor(int i_factor);

    bool showPopup() const;
    void setShowPopup(bool b_show);

    // Tesseract executable path. Empty = "tesseract" (rely on PATH).
    QString tesseractPath() const;
    void setTesseractPath(const QString& str_path);

    // Tessdata directory override. Empty = Tesseract default.
    QString tessdataDir() const;
    void setTessdataDir(const QString& str_dir);

    // OCR language(s), e.g., "chi_sim+eng".
    QString language() const;
    void setLanguage(const QString& str_lang);

    void load();
    void save();

    // Absolute path of the INI file (exe dir / clipocr.ini).
    static QString configFilePath();

    // True when the INI file does not exist yet (very first launch).
    bool firstRun() const;

    // Default OCR language based on system locale.
    static QString defaultLanguage();

private:

    QKeySequence m_SeqHotkey;
    int m_iUpscale = 3;
    bool m_bShowPopup = true;
    QString m_strTessPath;       // default: "tesseract"
    QString m_strTessdataDir;
    QString m_strLanguage;

    static const QString ORGANIZATION;
    static const QString APP_NAME;
};
