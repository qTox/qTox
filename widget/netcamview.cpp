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

#include "netcamview.h"

NetCamView::NetCamView(QWidget* parent)
    : QWidget(parent), displayLabel{new QLabel},
      mainLayout{new QHBoxLayout()}
{
    setLayout(mainLayout);
    setWindowTitle("Tox video");
    setMinimumSize(320,240);

    displayLabel->setScaledContents(true);

    mainLayout->addWidget(displayLabel);
}

void NetCamView::updateDisplay(vpx_image frame)
{
    int w = frame.d_w, h = frame.d_h;

    if (!frame.w || !frame.h || !w || !h)
        return;

    int bpl = frame.stride[VPX_PLANE_Y], cxbpl = frame.stride[VPX_PLANE_V];
    QImage img(w, h, QImage::Format_RGB32);

    uint8_t* yData = frame.planes[VPX_PLANE_Y];
    uint8_t* uData = frame.planes[VPX_PLANE_U];
    uint8_t* vData = frame.planes[VPX_PLANE_V];
    for (int i = 0; i< h; i++)
    {
        uint32_t* scanline = (uint32_t*)img.scanLine(i);
        for (int j=0; j < w; j++)
        {
            float Y = yData[i*bpl + j];
            float U = uData[i/2*cxbpl + j/2];
            float V = vData[i/2*cxbpl + j/2];

            uint8_t R = qMax(qMin((int)(Y + 1.402 * (V - 128)),255),0);
            uint8_t G = qMax(qMin((int)(Y - 0.344 * (U - 128) - 0.714 * (V - 128)),255),0);
            uint8_t B = qMax(qMin((int)(Y + 1.772 * (U - 128)),255),0);

            scanline[j] = (0xFF<<24) + (R<<16) + (G<<8) + B;
        }
    }

    vpx_img_free(&frame);
    displayLabel->setPixmap(QPixmap::fromImage(img));
}
