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
#include "camera.h"
#include <QCloseEvent>
#include <QShowEvent>
#include <QTimer>
#include <QLabel>
#include <QHBoxLayout>
#include <opencv2/opencv.hpp>

using namespace cv;

SelfCamView::SelfCamView(Camera* Cam, QWidget* parent)
    : QWidget(parent), displayLabel{new QLabel},
      mainLayout{new QHBoxLayout()}, cam(Cam), updateDisplayTimer{new QTimer}
{
    setLayout(mainLayout);
    setWindowTitle(SelfCamView::tr("Tox video test","Title of the window to test the video/webcam"));
    setMinimumSize(320,240);

    updateDisplayTimer->setInterval(5);
    updateDisplayTimer->setSingleShot(false);

    displayLabel->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(displayLabel);

    connect(updateDisplayTimer, SIGNAL(timeout()), this, SLOT(updateDisplay()));
}

void SelfCamView::closeEvent(QCloseEvent* event)
{
    cam->unsuscribe();
    updateDisplayTimer->stop();
    event->accept();
}

void SelfCamView::showEvent(QShowEvent* event)
{
    cam->suscribe();
    updateDisplayTimer->start();
    event->accept();
}

void SelfCamView::updateDisplay()
{
    displayLabel->setPixmap(QPixmap::fromImage(cam->getLastImage()).scaled(displayLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void SelfCamView::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e)
    updateDisplay();
}
