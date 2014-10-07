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
#include "ui_avsettings.h"

AVForm::AVForm(Camera* cam) :
    GenericForm(tr("Audio/Video settings"), QPixmap(":/img/settings/av.png"))
{
    bodyUI = new Ui::AVSettings;
    bodyUI->setupUi(this);

    camView = new SelfCamView(cam, this);
    bodyUI->videoGroup->layout()->addWidget(camView);

    auto modes = cam->getVideoModes();
    for (Camera::VideoMode m : modes)
    {
        bodyUI->videoModescomboBox->addItem(QString("%1x%2").arg(QString::number(m.res.width())
                                                                 ,QString::number(m.res.height())));
    }
}

AVForm::~AVForm()
{
    delete bodyUI;
}
