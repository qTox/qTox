/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

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

#include "netvideosource.h"

#include <QDebug>
#include <vpx/vpx_image.h>

NetVideoSource::NetVideoSource()
{
}

void NetVideoSource::pushFrame(VideoFrame frame)
{
    emit frameAvailable(frame);
}

void NetVideoSource::pushVPXFrame(vpx_image *image)
{
    int dw = image->d_w;
    int dh = image->d_h;

    int bpl = image->stride[VPX_PLANE_Y];
    int cxbpl = image->stride[VPX_PLANE_V];

    VideoFrame frame;
    frame.frameData.resize(dw * dh * 3); //YUV 24bit
    frame.resolution = QSize(dw, dh);
    frame.format = VideoFrame::YUV;

    uint8_t* yData = image->planes[VPX_PLANE_Y];
    uint8_t* uData = image->planes[VPX_PLANE_V];
    uint8_t* vData = image->planes[VPX_PLANE_U];

    for (int x = 0; x < dw; x += 1)
    {
        for (int y = 0; y < dh; y += 1)
        {
            uint8_t Y = yData[x + y * bpl];
            uint8_t U = uData[x/2 + y/2*cxbpl];
            uint8_t V = vData[x/2 + y/2*cxbpl];

            frame.frameData.data()[dw * 3 * y + x * 3 + 0] = Y;
            frame.frameData.data()[dw * 3 * y + x * 3 + 1] = U;
            frame.frameData.data()[dw * 3 * y + x * 3 + 2] = V;

        }
    }

    pushFrame(frame);
}
