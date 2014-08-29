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

#include "selfcamview.h"
#include <QActionGroup>
#include <QMessageBox>
#include <QCloseEvent>
#include <QShowEvent>
#include <QVideoFrame>

#include "videosurface.h"
#include "widget.h"

using namespace cv;

SelfCamView::SelfCamView(Camera* Cam, QWidget* parent)
    : QWidget(parent), displayLabel{new QLabel},
      mainLayout{new QHBoxLayout()}, cam(Cam)
{
    setLayout(mainLayout);
    setWindowTitle(SelfCamView::tr("Tox video test","Title of the window to test the video/webcam"));
    setMinimumSize(320,240);

    updateDisplayTimer.setInterval(5);
    updateDisplayTimer.setSingleShot(false);

    displayLabel->setScaledContents(true);

    mainLayout->addWidget(displayLabel);

    connect(&updateDisplayTimer, SIGNAL(timeout()), this, SLOT(updateDisplay()));
}

SelfCamView::~SelfCamView()
{
}

void SelfCamView::closeEvent(QCloseEvent* event)
{
    cam->unsuscribe();
    updateDisplayTimer.stop();
    event->accept();
}

void SelfCamView::showEvent(QShowEvent* event)
{
    cam->suscribe();
    updateDisplayTimer.start();
    event->accept();
}

QImage Mat2QImage(const cv::Mat3b &src) {
        QImage dest(src.cols, src.rows, QImage::Format_ARGB32);
        for (int y = 0; y < src.rows; ++y) {
                const cv::Vec3b *srcrow = src[y];
                QRgb *destrow = (QRgb*)dest.scanLine(y);
                for (int x = 0; x < src.cols; ++x) {
                        destrow[x] = qRgba(srcrow[x][2], srcrow[x][1], srcrow[x][0], 255);
                }
        }
        return dest;
}

void SelfCamView::updateDisplay()
{
    Mat frame = cam->getLastFrame();

    displayLabel->setPixmap(QPixmap::fromImage(Mat2QImage(frame)));
}

