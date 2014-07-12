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
#include "core.h"
#include "widget.h"
#include <QApplication>
#include <QtConcurrent/QtConcurrent>

static inline void fromYCbCrToRGB(
        uint8_t Y, uint8_t Cb, uint8_t Cr,
        uint8_t& R, uint8_t& G, uint8_t& B)
{
    int r = Y + ((1436 * (Cr - 128)) >> 10),
        g = Y - ((354 * (Cb - 128) + 732 * (Cr - 128)) >> 10),
        b = Y + ((1814 * (Cb - 128)) >> 10);

    if(r < 0) {
        r = 0;
    } else if(r > 255) {
        r = 255;
    }

    if(g < 0) {
        g = 0;
    } else if(g > 255) {
        g = 255;
    }

    if(b < 0) {
        b = 0;
    } else if(b > 255) {
        b = 255;
    }

    R = static_cast<uint8_t>(r);
    G = static_cast<uint8_t>(g);
    B = static_cast<uint8_t>(b);
}


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

void NetCamView::updateDisplay(vpx_image* frame)
{
    if (!frame->w || !frame->h)
        return;

    Core* core = Widget::getInstance()->getCore();

    core->increaseVideoBusyness();

    QImage img = convert(*frame);

    vpx_img_free(frame);
    displayLabel->setPixmap(QPixmap::fromImage(img));

    core->decreaseVideoBusyness();
}

QImage NetCamView::convert(vpx_image& frame)
{
    int w = frame.d_w, h = frame.d_h;
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
            uint8_t Y = yData[i*bpl + j];
            uint8_t U = uData[i/2*cxbpl + j/2];
            uint8_t V = vData[i/2*cxbpl + j/2];

            uint8_t R, G, B;
            fromYCbCrToRGB(Y, U, V, R, G, B);

            scanline[j] = (0xFF<<24) + (R<<16) + (G<<8) + B;
        }
    }

    return img;
}
