#pragma once

#include <QImage>

// ClipOCR - clipboard OCR utility.
// Image pre-processing helpers for OCR.
class ImageProcessor
{
public:
    // Upscale an image by `i_factor` using smooth (bicubic-like) interpolation.
    // i_factor <= 1 (or a null image) returns the source unchanged.
    static QImage upscale(const QImage& src, int i_factor);
};
