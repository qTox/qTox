/*
    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#include "videoframe.h"

vpx_image_t VideoFrame::createVpxImage() const
{
    vpx_image img;
    img.w = img.h = img.d_w = img.d_h = 0;

    if (!isValid())
        return img;

    const int w = resolution.width();
    const int h = resolution.height();

    // I420 "It comprises an NxM Y plane followed by (N/2)x(M/2) V and U planes."
    // http://fourcc.org/yuv.php#IYUV
    vpx_img_alloc(&img, VPX_IMG_FMT_VPXI420, w, h, 1);

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            uint8_t b = frameData.data()[(x + y * w) * 3 + 0];
            uint8_t g = frameData.data()[(x + y * w) * 3 + 1];
            uint8_t r = frameData.data()[(x + y * w) * 3 + 2];

            img.planes[VPX_PLANE_Y][x + y * img.stride[VPX_PLANE_Y]] = ((66 * r + 129 * g + 25 * b) >> 8) + 16;

            if (!(x % (1 << img.x_chroma_shift)) && !(y % (1 << img.y_chroma_shift)))
            {
                const int i = x / (1 << img.x_chroma_shift);
                const int j = y / (1 << img.y_chroma_shift);

                img.planes[VPX_PLANE_V][i + j * img.stride[VPX_PLANE_V]] = ((112 * r + -94 * g + -18 * b) >> 8) + 128;
                img.planes[VPX_PLANE_U][i + j * img.stride[VPX_PLANE_U]] = ((-38 * r + -74 * g + 112 * b) >> 8) + 128;
            }
        }
    }

    return img;
}
