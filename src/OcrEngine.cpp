// ClipOCR - clipboard OCR utility.
// OCR engine using Tesseract CLI (QProcess). Simple, reliable, no WinRT.

#include "OcrEngine.h"
#include "ImageProcessor.h"

#include <QCoreApplication>
#include <QDir>
#include <QImage>
#include <QProcess>
#include <QTemporaryFile>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QThread>
#include <QMutex>

void OcrEngine::log(const QString& strMsg)
{
    // Debug logging is opt-in via the CLIPOCR_DEBUG environment variable.
    // When unset, this is a no-op so normal runs stay clean (no clipocr.log).
    static const bool s_bEnabled = !qEnvironmentVariableIsEmpty("CLIPOCR_DEBUG");
    if (!s_bEnabled)
        return;

    static QMutex s_mutex;
    QMutexLocker locker(&s_mutex);
    const QString strPath = QCoreApplication::applicationDirPath()
                            + QStringLiteral("/clipocr.log");
    QFile file(strPath);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream ts(&file);
        ts << QDateTime::currentDateTime().toString(QStringLiteral("hh:mm:ss.zzz"))
           << QStringLiteral(" [T") << reinterpret_cast<quintptr>(QThread::currentThreadId())
           << QStringLiteral("] ") << strMsg << QStringLiteral("\n");
        file.flush();
    }
}

OcrEngine::OcrEngine(QObject* parent) : QObject(parent)
{
    m_strTesseractPath = QStringLiteral("tesseract");
}

void OcrEngine::setTesseractPath(const QString& str_path)
{
    m_strTesseractPath = str_path.trimmed();
    if (m_strTesseractPath.isEmpty())
        m_strTesseractPath = QStringLiteral("tesseract");
}

void OcrEngine::setTessdataDir(const QString& str_dir) { m_strTessdataDir = str_dir; }
void OcrEngine::setLanguage(const QString& str_lang) { m_strLanguage = str_lang; }
void OcrEngine::setUpscaleFactor(int i_factor) { m_iUpscale = qBound(1, i_factor, 4); }

bool OcrEngine::isReady() const
{
    // We can only verify at runtime that tesseract is reachable.
    // Return true if the path is non-empty; actual errors show in recognize().
    return !m_strTesseractPath.isEmpty();
}

QString OcrEngine::postProcess(const QString& str_raw)
{
    QString strText = str_raw.trimmed();
    QString strOut;
    strOut.reserve(strText.size());
    for (int i = 0; i < strText.size(); ++i) {
        QChar c = strText.at(i);
        // Remove spaces inserted by Tesseract between two CJK characters.
        if (c == QChar(' ') && i > 0 && i + 1 < strText.size()) {
            const bool bPrevNonAscii = strText.at(i - 1).unicode() > 0x7F;
            const bool bNextNonAscii = strText.at(i + 1).unicode() > 0x7F;
            if (bPrevNonAscii && bNextNonAscii)
                continue;
        }
        strOut.append(c);
    }
    return strOut;
}

bool OcrEngine::recognize(const QImage& image, QString& strOutText, QString& strOutError)
{
    strOutText.clear();
    strOutError.clear();

    log(QStringLiteral("recognize: enter, img=%1x%2 upscale=%3 lang=%4")
        .arg(image.width()).arg(image.height()).arg(m_iUpscale).arg(m_strLanguage));

    if (image.isNull()) {
        strOutError = tr("Empty image");
        log(QStringLiteral("recognize: empty image, abort"));
        return false;
    }

    // 1) Upscale to improve small-text recognition.
    const QImage imgWork = ImageProcessor::upscale(image, m_iUpscale);
    log(QStringLiteral("recognize: upscaled to %1x%2")
        .arg(imgWork.width()).arg(imgWork.height()));

    // 2) Write a temp PNG for Tesseract.
    QTemporaryFile tmpFile(QDir::tempPath() + QStringLiteral("/clipocr_XXXXXX.png"));
    if (!tmpFile.open()) {
        strOutError = tr("Failed to create temporary file.");
        log(QStringLiteral("recognize: cannot open temp file, abort"));
        return false;
    }
    const QString strTempPng = tmpFile.fileName();
    tmpFile.close();
    if (!imgWork.save(strTempPng, "PNG")) {
        strOutError = tr("Failed to write temporary image.");
        log(QStringLiteral("recognize: cannot save temp png, abort"));
        return false;
    }
    log(QStringLiteral("recognize: temp png = %1").arg(strTempPng));

    // 3) Build the Tesseract command line.
    QStringList listArgs;

    // Set tessdata directory if configured (--tessdata-dir).
    if (!m_strTessdataDir.isEmpty())
        listArgs << QStringLiteral("--tessdata-dir") << m_strTessdataDir;

    // Page segmentation mode: assume a single uniform block of text.
    listArgs << QStringLiteral("--psm") << QStringLiteral("6");

    // OCR engine mode: use LSTM-based neural network (best quality).
    // NOTE: must be "--oem" (double dash). A single dash is NOT recognized
    // by Tesseract and gets mis-parsed as the input image path.
    listArgs << QStringLiteral("--oem") << QStringLiteral("1");

    // Language(s).
    if (!m_strLanguage.isEmpty())
        listArgs << QStringLiteral("-l") << m_strLanguage;

    // Output to a temp text file (robust alternative to '-' for stdout).
    QTemporaryFile outTmp(QDir::tempPath() + QStringLiteral("/clipocr_outXXXXXX"));
    if (!outTmp.open()) {
        strOutError = tr("Failed to create temporary output file.");
        log(QStringLiteral("recognize: cannot open temp out file, abort"));
        return false;
    }
    const QString strOutBase = outTmp.fileName();
    outTmp.remove();   // Tesseract will create <base>.txt next to it
    const QString strOutTxt = strOutBase + QStringLiteral(".txt");

    // Input image, then output base (Tesseract appends .txt).
    listArgs << strTempPng << strOutBase;

    // 4) Run Tesseract.
    QProcess proc;
    proc.start(m_strTesseractPath, listArgs);
    log(QStringLiteral("recognize: starting tesseract path=[%1]").arg(m_strTesseractPath));

    if (!proc.waitForStarted(10000)) {
        strOutError = tr("Failed to start Tesseract: ") + m_strTesseractPath
                    + tr("\nMake sure Tesseract is installed and on PATH, "
                         "or set the correct path in Settings.");
        log(QStringLiteral("recognize: waitForStarted FAILED -> %1").arg(strOutError));
        return false;
    }
    log(QStringLiteral("recognize: tesseract started OK"));

    const bool bFinished = proc.waitForFinished(60000);
    const QByteArray baStdErr = proc.readAllStandardError();

    if (!bFinished) {
        proc.kill();
        QFile::remove(strOutTxt);
        strOutError = tr("OCR timed out (60s).");
        log(QStringLiteral("recognize: TIMEOUT after 60s"));
        return false;
    }

    if (proc.exitCode() != 0) {
        QFile::remove(strOutTxt);
        strOutError = tr("OCR failed (exit code %1): ").arg(proc.exitCode())
                    + QString::fromLocal8Bit(baStdErr).trimmed()
                    + tr("\nCheck that the language data (e.g., chi_sim.traineddata) "
                         "is available in the tessdata directory.");
        log(QStringLiteral("recognize: exitCode=%1 stderr=[%2]")
            .arg(proc.exitCode()).arg(QString::fromLocal8Bit(baStdErr).trimmed()));
        return false;
    }

    // 5) Read the result text file and post-process.
    QFile fileOut(strOutTxt);
    if (!fileOut.open(QIODevice::ReadOnly | QIODevice::Text)) {
        strOutError = tr("Failed to read OCR result.");
        log(QStringLiteral("recognize: cannot open result txt=[%1]").arg(strOutTxt));
        return false;
    }
    QTextStream tsOut(&fileOut);
    strOutText = postProcess(tsOut.readAll());
    fileOut.close();
    QFile::remove(strOutTxt);

    log(QStringLiteral("recognize: SUCCESS, outChars=%1").arg(strOutText.size()));
    return true;
}
