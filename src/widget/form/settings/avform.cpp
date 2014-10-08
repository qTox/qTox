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
#include "src/widget/camera.h"
#include "ui_avsettings.h"

AVForm::AVForm(Camera* cam) :
    GenericForm(tr("Audio/Video settings"), QPixmap(":/img/settings/av.png"))
{
    bodyUI = new Ui::AVSettings;
    bodyUI->setupUi(this);

    camView = new SelfCamView(cam, this);
    bodyUI->videoGroup->layout()->addWidget(camView);
    camView->hide(); // hide by default

    connect(bodyUI->testVideoBtn, &QPushButton::clicked, this, &AVForm::onTestVideoPressed);
}

AVForm::~AVForm()
{
    delete bodyUI;
}

void AVForm::showTestVideo()
{
    bodyUI->testVideoBtn->setText(tr("Hide video preview","On a button"));
    camView->show();
}

void AVForm::closeTestVideo()
{
    bodyUI->testVideoBtn->setText(tr("Show video preview","On a button"));
    camView->close();
}

void AVForm::onTestVideoPressed()
{
    if (camView->isVisible())
        closeTestVideo();
    else
        showTestVideo();
}
