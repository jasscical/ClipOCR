#pragma once

#include <QImage>

// ClipOCR - 剪贴板 OCR 小工具。
// 供 OCR 使用的图像预处理辅助函数。
class ImageProcessor
{
public:
    // 按 i_factor 放大图片，使用平滑（类双三次）插值。
    // i_factor <= 1（或空图片）时原样返回。
    static QImage upscale(const QImage& src, int i_factor);
};
