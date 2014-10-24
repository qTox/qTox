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
#include "ui_avsettings.h"

AVForm::AVForm() :
    GenericForm(tr("Audio/Video"), QPixmap(":/img/settings/av.png"))
{
    bodyUI = new Ui::AVSettings;
    bodyUI->setupUi(this);

    connect(Camera::getInstance(), &Camera::propProbingFinished, this, &AVForm::onPropProbingFinished);
    connect(Camera::getInstance(), &Camera::resolutionProbingFinished, this, &AVForm::onResProbingFinished);
}

AVForm::~AVForm()
{
    delete bodyUI;
}

void AVForm::present()
{
    bodyUI->CamVideoSurface->setSource(Camera::getInstance());

    Camera::getInstance()->probeProp(Camera::SATURATION);
    Camera::getInstance()->probeProp(Camera::CONTRAST);
    Camera::getInstance()->probeProp(Camera::BRIGHTNESS);
    Camera::getInstance()->probeProp(Camera::HUE);

    Camera::getInstance()->probeResolutions();
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

void AVForm::on_videoModescomboBox_activated(int index)
{
    Camera::getInstance()->setResolution(bodyUI->videoModescomboBox->itemData(index).toSize());
}

void AVForm::onPropProbingFinished(Camera::Prop prop, double val)
{
    switch (prop)
    {
    case Camera::BRIGHTNESS:
        bodyUI->BrightnessSlider->setValue(val*100);
        break;
    case Camera::CONTRAST:
        bodyUI->ContrastSlider->setValue(val*100);
        break;
    case Camera::SATURATION:
        bodyUI->SaturationSlider->setValue(val*100);
        break;
    case Camera::HUE:
        bodyUI->HueSlider->setValue(val*100);
        break;
    default:
        break;
    }
}

void AVForm::onResProbingFinished(QList<QSize> res)
{
    bodyUI->videoModescomboBox->clear();
    for (QSize r : res)
        bodyUI->videoModescomboBox->addItem(QString("%1x%2").arg(QString::number(r.width()),QString::number(r.height())), r);

    bodyUI->videoModescomboBox->setCurrentIndex(bodyUI->videoModescomboBox->count()-1);
}

void AVForm::hideEvent(QHideEvent *)
{
    bodyUI->CamVideoSurface->setSource(nullptr);
}
