#pragma once

#include <QObject>
#include <QImage>

// ClipOCR - 剪贴板 OCR 小工具。
// 按需读取当前剪贴板图片（由全局热键触发）。
class ClipboardMonitor : public QObject
{
    Q_OBJECT
public:
    explicit ClipboardMonitor(QObject* parent = nullptr);

public slots:
    // 抓取当前剪贴板图片并发出结果信号。
    void grabFromClipboard();

signals:
    void imageCaptured(const QImage& image);
    void noImageInClipboard();
};
