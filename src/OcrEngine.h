// ClipOCR - 剪贴板 OCR 小工具。
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

    // 可选的调试日志（位于可执行文件同目录的 clipocr.log）。
    // 仅当设置了 CLIPOCR_DEBUG 环境变量时才启用。
    static void log(const QString& strMsg);

    // 对一张图片执行 OCR。成功时返回 true，识别文字写入 strOutText；
    // 失败时返回 false，人类可读的错误信息写入 strOutError。
    bool recognize(const QImage& image, QString& strOutText, QString& strOutError);

private:
    static QString postProcess(const QString& str_raw);

    // Tesseract 可执行文件名/路径。默认："tesseract"（依赖 PATH）。
    QString m_strTesseractPath;
    // Tessdata 目录覆盖。留空 = Tesseract 默认目录。
    QString m_strTessdataDir;
    // 传给 -l 的语言字符串（如 "chi_sim+eng"）。
    QString m_strLanguage;
    // 放大倍数（1-4）。越大对小字识别越准，但更耗时。
    int m_iUpscale = 3;
};
