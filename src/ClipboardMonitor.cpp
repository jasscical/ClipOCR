#include "ClipboardMonitor.h"

#include <QApplication>
#include <QClipboard>

ClipboardMonitor::ClipboardMonitor(QObject* parent) : QObject(parent) {}

void ClipboardMonitor::grabFromClipboard()
{
    QClipboard* p_cb = QApplication::clipboard();
    if (!p_cb)
        return;

    // QClipboard::image() works across Windows / macOS / Linux.
    const QImage img = p_cb->image();
    if (img.isNull())
        emit noImageInClipboard();
    else
        emit imageCaptured(img);
}
