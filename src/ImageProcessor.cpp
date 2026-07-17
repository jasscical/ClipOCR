#include "ImageProcessor.h"

QImage ImageProcessor::upscale(const QImage& src, int i_factor)
{
    if (i_factor <= 1 || src.isNull())
        return src;

    const int i_w = src.width() * i_factor;
    const int i_h = src.height() * i_factor;
    return src.scaled(i_w, i_h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}
