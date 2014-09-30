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

#include "avform.h"
#include "widget/camera.h"

AVForm::AVForm(Camera* cam)
{
    icon.setPixmap(QPixmap(":/img/settings/av.png").scaledToHeight(headLayout.sizeHint().height(), Qt::SmoothTransformation));
    label.setText(tr("Audio/Video settings"));
    
    QGroupBox *group = new QGroupBox(tr("Video Settings"));

    camView = new SelfCamView(cam);
    camView->hide(); // hide by default
    testVideo = new QPushButton(tr("Show video preview","On a button"));
    connect(testVideo, &QPushButton::clicked, this, &AVForm::onTestVideoPressed);

    videoLayout = new QVBoxLayout();
    videoLayout->addWidget(testVideo);
    videoLayout->addWidget(camView);
    //videoGroup->setLayout(videoLayout); // strangely enough, this causes a segfault....

    layout.addWidget(group);
    layout.addStretch(1);
}

AVForm::~AVForm()
{
}

void AVForm::showTestVideo()
{
    testVideo->setText(tr("Hide video preview","On a button"));
    camView->show();
}

void AVForm::closeTestVideo()
{
    testVideo->setText(tr("Show video preview","On a button"));
    camView->close();
}

void AVForm::onTestVideoPressed()
{
    if (camView->isVisible())
        closeTestVideo();
    else
        showTestVideo();
}
