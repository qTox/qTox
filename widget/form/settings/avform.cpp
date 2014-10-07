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

AVForm::AVForm(Camera* Cam) :
    GenericForm(tr("Audio/Video settings"), QPixmap(":/img/settings/av.png")), cam(Cam)
{
    bodyUI = new Ui::AVSettings;
    bodyUI->setupUi(this);

    cam->subscribe();
    cam->setVideoMode(cam->getBestVideoMode());
    camView = new SelfCamView(cam, this);

    bodyUI->videoGroup->layout()->addWidget(camView);

    auto modes = cam->getVideoModes();
    for (Camera::VideoMode m : modes)
    {
        bodyUI->videoModescomboBox->addItem(QString("%1x%2").arg(QString::number(m.res.width())
                                                                 ,QString::number(m.res.height())));
    }

    bodyUI->ContrastSlider->setValue(cam->getProp(Camera::CONTRAST)*100);
    bodyUI->BrightnessSlider->setValue(cam->getProp(Camera::BRIGHTNESS)*100);
    bodyUI->SaturationSlider->setValue(cam->getProp(Camera::SATURATION)*100);
    bodyUI->HueSlider->setValue(cam->getProp(Camera::HUE)*100);
}

AVForm::~AVForm()
{
    delete bodyUI;
}

void AVForm::on_ContrastSlider_sliderMoved(int position)
{
    cam->setProp(Camera::CONTRAST, position / 100.0);
}

void AVForm::on_SaturationSlider_sliderMoved(int position)
{
    cam->setProp(Camera::SATURATION, position / 100.0);
}

void AVForm::on_BrightnessSlider_sliderMoved(int position)
{
    cam->setProp(Camera::BRIGHTNESS, position / 100.0);
}

void AVForm::on_HueSlider_sliderMoved(int position)
{
    cam->setProp(Camera::HUE, position / 100.0);
}

void AVForm::on_videoModescomboBox_currentIndexChanged(const QString &arg1)
{
    QStringList resStr = arg1.split("x");
    int w = resStr[0].toInt();
    int h = resStr[0].toInt();

    cam->setVideoMode(Camera::VideoMode{QSize(w,h),60});
}
