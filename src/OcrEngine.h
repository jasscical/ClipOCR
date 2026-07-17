// ClipOCR - clipboard OCR utility.
#pragma once

#include <QObject>
#include <QImage>
#include <QString>

class OcrEngine : public QObject
{
    Q_OBJECT

public:
    explicit OcrEngine(QObject* parent = nullptr);

    void setTesseractPath(const QString& str_path);
    void setTessdataDir(const QString& str_dir);
    void setLanguage(const QString& str_lang);
    void setUpscaleFactor(int i_factor);

    bool isReady() const;

    // Optional debug log (clipocr.log next to the executable).
    // Enabled only when the CLIPOCR_DEBUG environment variable is set.
    static void log(const QString& strMsg);

    // Run OCR on an image. Returns true on success with recognized text in
    // strOutText, or false with a human-readable error in strOutError.
    bool recognize(const QImage& image, QString& strOutText, QString& strOutError);

private:
    static QString postProcess(const QString& str_raw);

    // Tesseract executable name/path. Default: "tesseract" (relies on PATH).
    QString m_strTesseractPath;
    // Tessdata directory override. Empty = Tesseract default.
    QString m_strTessdataDir;
    // Language string passed to -l (e.g., "chi_sim+eng").
    QString m_strLanguage;
    // Upscale factor (1-4). Higher improves small-text accuracy at cost of time.
    int m_iUpscale = 3;
};
