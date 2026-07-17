#pragma once

#include <QObject>
#include <QImage>

// ClipOCR - clipboard OCR utility.
// Reads the current clipboard image on demand (triggered by the global hotkey).
class ClipboardMonitor : public QObject
{
    Q_OBJECT
public:
    explicit ClipboardMonitor(QObject* parent = nullptr);

public slots:
    // Grab the current clipboard image and emit the result.
    void grabFromClipboard();

signals:
    void imageCaptured(const QImage& image);
    void noImageInClipboard();
};
