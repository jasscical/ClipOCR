#pragma once

#include <QObject>
#include <QKeySequence>
#include <QString>

// ClipOCR - 剪贴板 OCR 小工具。
// 持久化设置，保存在 exe 同目录下的 INI 文件（clipocr.ini）。
// 参见 docs/CODING_STYLE.md。
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

    // Tesseract 可执行文件路径。留空 = "tesseract"（依赖 PATH 查找）。
    QString tesseractPath() const;
    void setTesseractPath(const QString& str_path);

    // Tessdata 目录覆盖。留空 = Tesseract 默认目录。
    QString tessdataDir() const;
    void setTessdataDir(const QString& str_dir);

    // 界面显示语言代码："en" 或 "zh_CN"。
    QString language() const;
    void setLanguage(const QString& str_lang);

    // 传给 Tesseract 的 OCR 识别语言（图片内文字语种），如 "chi_sim+eng"。
    // 这是「图片中文字」的语种，与上面的界面语言相互独立，
    // 用户可在设置对话框中修改。
    QString ocrLanguage() const;
    void setOcrLanguage(const QString& str_lang);

    void load();
    void save();

    // INI 文件的绝对路径（exe 所在目录 / clipocr.ini）。
    static QString configFilePath();

    // 当 INI 文件尚不存在时为 true（首次启动）。
    bool firstRun() const;

    // 依据系统区域返回的默认界面语言（中文环境为 "zh_CN"，否则 "en"）。
    static QString defaultLanguage();

    // 图片文字的默认 OCR 语言：中文环境为 "chi_sim+eng"，
    // 否则为 "eng"。
    static QString defaultOcrLanguage();

private:

    QKeySequence m_SeqHotkey;
    int m_iUpscale = 3;
    bool m_bShowPopup = true;
    QString m_strTessPath;       // 默认："tesseract"
    QString m_strTessdataDir;
    QString m_strLanguage;
    QString m_strOcrLanguage;

    static const QString ORGANIZATION;
    static const QString APP_NAME;
};
