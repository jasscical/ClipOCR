// ClipOCR - 剪贴板 OCR 小工具。
// OCR 引擎，调用 Tesseract 命令行（QProcess）。简单可靠，不依赖 WinRT。

#include "OcrEngine.h"
#include "ImageProcessor.h"

#include <QCoreApplication>
#include <QDir>
#include <QImage>
#include <QProcess>
#include <QTemporaryFile>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDateTime>
#include <QThread>
#include <QMutex>

void OcrEngine::log(const QString& strMsg)
{
    // 调试日志通过 CLIPOCR_DEBUG 环境变量按需开启。
    // 未设置时本函数为空操作，正常运行不产生 clipocr.log，保持干净。
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

// 解析 tessdata 目录，用于预先检查语言包是否齐全。
// 无法确定时返回空（例如 PATH 上的裸 "tesseract"），此时跳过检查，
// 改由 Tesseract 自行报告缺失数据的错误。
static QString resolveTessdataDir(const QString& str_tessPath, const QString& str_override)
{
    if (!str_override.isEmpty())
        return str_override;
    const QFileInfo info(str_tessPath);
    if (info.isAbsolute()
        && info.fileName().compare(QStringLiteral("tesseract"), Qt::CaseInsensitive) == 0) {
        return info.absolutePath() + QStringLiteral("/tessdata");
    }
    return QString();
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
    // 只能在运行时确认 tesseract 是否可用。
    // 路径非空即返回 true；真正的错误会在 recognize() 中暴露。
    return !m_strTesseractPath.isEmpty();
}

QString OcrEngine::postProcess(const QString& str_raw)
{
    QString strText = str_raw.trimmed();
    QString strOut;
    strOut.reserve(strText.size());
    for (int i = 0; i < strText.size(); ++i) {
        QChar c = strText.at(i);
        // 去掉 Tesseract 在两个 CJK 字符之间插入的空格。
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

    // 0) 若已知 tessdata 位置，先预检语言包，这样缺少 .traineddata 时
    //     给出清晰提示，而不是原始的 stderr 报错。
    const QString strTessDir = resolveTessdataDir(m_strTesseractPath, m_strTessdataDir);
    if (!strTessDir.isEmpty()) {
        const QStringList listCodes =
            m_strLanguage.split(QLatin1Char('+'), Qt::SkipEmptyParts);
        for (const QString& strCode : listCodes) {
            const QString strFile = strTessDir + QStringLiteral("/") + strCode
                                    + QStringLiteral(".traineddata");
            if (!QFile::exists(strFile)) {
                strOutError = tr("Missing language data: %1. Download the "
                                 "corresponding .traineddata into the tessdata directory.")
                    .arg(strCode + QStringLiteral(".traineddata"));
                log(QStringLiteral("recognize: missing language data %1").arg(strFile));
                return false;
            }
        }
    }

    // 1) 放大图片，提升小字识别率。
    const QImage imgWork = ImageProcessor::upscale(image, m_iUpscale);
    log(QStringLiteral("recognize: upscaled to %1x%2")
        .arg(imgWork.width()).arg(imgWork.height()));

    // 2) 写一个临时 PNG 供 Tesseract 读取。
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

    // 3) 组装 Tesseract 命令行。
    QStringList listArgs;

    // 若设置了 tessdata 目录，则加入 --tessdata-dir。
    if (!m_strTessdataDir.isEmpty())
        listArgs << QStringLiteral("--tessdata-dir") << m_strTessdataDir;

    // 页面分割模式：假定为单块统一文本。
    listArgs << QStringLiteral("--psm") << QStringLiteral("6");

    // OCR 引擎模式：使用基于 LSTM 的神经网络（质量最佳）。
    // 注意：必须是 "--oem"（双横线）。单横线 Tesseract 不识别，
    // 会被误解析为输入图片路径。
    listArgs << QStringLiteral("--oem") << QStringLiteral("1");

    // 语言（可多个，用 + 连接）。
    if (!m_strLanguage.isEmpty())
        listArgs << QStringLiteral("-l") << m_strLanguage;

    // 输出到临时文本文件（比用 '-' 表示 stdout 更稳妥）。
    QTemporaryFile outTmp(QDir::tempPath() + QStringLiteral("/clipocr_outXXXXXX"));
    if (!outTmp.open()) {
        strOutError = tr("Failed to create temporary output file.");
        log(QStringLiteral("recognize: cannot open temp out file, abort"));
        return false;
    }
    const QString strOutBase = outTmp.fileName();
    outTmp.remove();   // Tesseract 会在同目录创建 <base>.txt
    const QString strOutTxt = strOutBase + QStringLiteral(".txt");

    // 输入图片，然后是输出基名（Tesseract 会自动补 .txt）。
    listArgs << strTempPng << strOutBase;

    // 4) 运行 Tesseract。
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

    // 5) 读取结果文本文件并做后处理。
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
