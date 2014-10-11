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

AVForm::AVForm() :
    GenericForm(tr("Audio/Video settings"), QPixmap(":/img/settings/av.png"))
{
    bodyUI = new Ui::AVSettings;
    bodyUI->setupUi(this);

    bodyUI->CamVideoSurface->setSource(Camera::getInstance());
}

AVForm::~AVForm()
{
    delete bodyUI;
}

void AVForm::present()
{
    bodyUI->videoModescomboBox->clear();
    QList<QSize> res = Camera::getInstance()->getSupportedResolutions();
    for (QSize r : res)
        bodyUI->videoModescomboBox->addItem(QString("%1x%2").arg(QString::number(r.width()),QString::number(r.height())));

    bodyUI->ContrastSlider->setValue(Camera::getInstance()->getProp(Camera::CONTRAST)*100);
    bodyUI->BrightnessSlider->setValue(Camera::getInstance()->getProp(Camera::BRIGHTNESS)*100);
    bodyUI->SaturationSlider->setValue(Camera::getInstance()->getProp(Camera::SATURATION)*100);
    bodyUI->HueSlider->setValue(Camera::getInstance()->getProp(Camera::HUE)*100);
}

void AVForm::on_ContrastSlider_sliderMoved(int position)
{
    Camera::getInstance()->setProp(Camera::CONTRAST, position / 100.0);
}

void AVForm::on_SaturationSlider_sliderMoved(int position)
{
    Camera::getInstance()->setProp(Camera::SATURATION, position / 100.0);
}

void AVForm::on_BrightnessSlider_sliderMoved(int position)
{
    Camera::getInstance()->setProp(Camera::BRIGHTNESS, position / 100.0);
}

void AVForm::on_HueSlider_sliderMoved(int position)
{
    Camera::getInstance()->setProp(Camera::HUE, position / 100.0);
}

void AVForm::on_videoModescomboBox_currentIndexChanged(const QString &arg1)
{
    QStringList resStr = arg1.split("x");
    int w = resStr[0].toInt();
    int h = resStr[0].toInt();

    Camera::getInstance()->setResolution(QSize(w,h));
}
