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
#include "src/misc/settings.h"
#include "src/audio.h"

#if defined(__APPLE__) && defined(__MACH__)
 #include <OpenAL/al.h>
 #include <OpenAL/alc.h>
#else
 #include <AL/alc.h>
 #include <AL/al.h>
#endif

#ifndef ALC_ALL_DEVICES_SPECIFIER
#define ALC_ALL_DEVICES_SPECIFIER ALC_DEVICE_SPECIFIER
#endif

AVForm::AVForm() :
    GenericForm(tr("Audio/Video"), QPixmap(":/img/settings/av.png")),
    CamVideoSurface{nullptr}
{
    bodyUI = new Ui::AVSettings;
    bodyUI->setupUi(this);

#ifdef QTOX_FILTER_AUDIO
    bodyUI->filterAudio->setChecked(Settings::getInstance().getFilterAudio());
#else
    bodyUI->filterAudio->setDisabled(true);
#endif

    connect(Camera::getInstance(), &Camera::propProbingFinished, this, &AVForm::onPropProbingFinished);
    connect(Camera::getInstance(), &Camera::resolutionProbingFinished, this, &AVForm::onResProbingFinished);

    auto qcomboboxIndexChanged = (void(QComboBox::*)(const QString&)) &QComboBox::currentIndexChanged;
    connect(bodyUI->inDevCombobox, qcomboboxIndexChanged, this, &AVForm::onInDevChanged);
    connect(bodyUI->outDevCombobox, qcomboboxIndexChanged, this, &AVForm::onOutDevChanged);
    connect(bodyUI->filterAudio, &QCheckBox::toggled, this, &AVForm::onFilterAudioToggled);
    connect(bodyUI->rescanButton, &QPushButton::clicked, this, [=](){getAudioInDevices(); getAudioOutDevices();});
    bodyUI->playbackSlider->setValue(100);
    bodyUI->microphoneSlider->setValue(100);
    bodyUI->playbackSlider->setEnabled(false);
    bodyUI->microphoneSlider->setEnabled(false);

    for (QComboBox* cb : findChildren<QComboBox*>())
    {
        cb->installEventFilter(this);
        cb->setFocusPolicy(Qt::StrongFocus);
    }
}

AVForm::~AVForm()
{
    delete bodyUI;
}

void AVForm::present()
{
    getAudioOutDevices();
    getAudioInDevices();

    createVideoSurface();
    CamVideoSurface->setSource(Camera::getInstance());

    Camera::getInstance()->probeProp(Camera::SATURATION);
    Camera::getInstance()->probeProp(Camera::CONTRAST);
    Camera::getInstance()->probeProp(Camera::BRIGHTNESS);
    Camera::getInstance()->probeProp(Camera::HUE);
    Camera::getInstance()->probeResolutions();
	
	bodyUI->videoModescomboBox->blockSignals(true);
	bodyUI->videoModescomboBox->addItem(tr("Initializing Camera..."));
	bodyUI->videoModescomboBox->blockSignals(false);
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

void AVForm::on_videoModescomboBox_currentIndexChanged(int index)
{
    QSize res = bodyUI->videoModescomboBox->itemData(index).toSize();
    Settings::getInstance().setCamVideoRes(res);
    Camera::getInstance()->setResolution(res);
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
    QSize savedRes = Settings::getInstance().getCamVideoRes();
    int savedResIndex = -1;
    bodyUI->videoModescomboBox->clear();
	bodyUI->videoModescomboBox->blockSignals(true);
    for (int i=0; i<res.size(); ++i)
    {
        QSize& r = res[i];
        bodyUI->videoModescomboBox->addItem(QString("%1x%2").arg(QString::number(r.width()),QString::number(r.height())), r);
        if (r == savedRes)
            savedResIndex = i;
    }
    //reset index, otherwise cameras with only one resolution won't get initialized
    bodyUI->videoModescomboBox->setCurrentIndex(-1);
    bodyUI->videoModescomboBox->blockSignals(false);

    if (savedResIndex != -1)
        bodyUI->videoModescomboBox->setCurrentIndex(savedResIndex);
    else
        bodyUI->videoModescomboBox->setCurrentIndex(bodyUI->videoModescomboBox->count()-1);
}

void AVForm::hideEvent(QHideEvent *)
{
    if (CamVideoSurface)
    {
        CamVideoSurface->setSource(nullptr);
        killVideoSurface();
    }
}

void AVForm::showEvent(QShowEvent *)
{
    createVideoSurface();
    CamVideoSurface->setSource(Camera::getInstance());
}

void AVForm::getAudioInDevices()
{
    QString settingsInDev = Settings::getInstance().getInDev();
	int inDevIndex = 0;
    bodyUI->inDevCombobox->clear();
    const ALchar *pDeviceList = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
    if (pDeviceList)
    {
		//prevent currentIndexChanged to be fired while adding items
		bodyUI->inDevCombobox->blockSignals(true);
        while (*pDeviceList)
        {
            int len = strlen(pDeviceList);
#ifdef Q_OS_WIN32			
            QString inDev = QString::fromUtf8(pDeviceList,len);
#else
			QString inDev = QString::fromLocal8Bit(pDeviceList,len);
#endif
            bodyUI->inDevCombobox->addItem(inDev);
            if (settingsInDev == inDev)
            {
				inDevIndex = bodyUI->inDevCombobox->count()-1;
            }
            pDeviceList += len+1;
        }
		//addItem changes currentIndex -> reset
		bodyUI->inDevCombobox->setCurrentIndex(-1);
		bodyUI->inDevCombobox->blockSignals(false);
    }
	bodyUI->inDevCombobox->setCurrentIndex(inDevIndex);
}

void AVForm::getAudioOutDevices()
{
    QString settingsOutDev = Settings::getInstance().getOutDev();
	int outDevIndex = 0;
    bodyUI->outDevCombobox->clear();
    const ALchar *pDeviceList;
    if (alcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT") != AL_FALSE)
        pDeviceList = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
    else
        pDeviceList = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
    if (pDeviceList)
    {
		//prevent currentIndexChanged to be fired while adding items
		bodyUI->outDevCombobox->blockSignals(true);
        while (*pDeviceList)
        {
            int len = strlen(pDeviceList);
#ifdef Q_OS_WIN32			
            QString outDev = QString::fromUtf8(pDeviceList,len);
#else
			QString outDev = QString::fromLocal8Bit(pDeviceList,len);
#endif
            bodyUI->outDevCombobox->addItem(outDev);
            if (settingsOutDev == outDev)
            {
                outDevIndex = bodyUI->outDevCombobox->count()-1;
            }
            pDeviceList += len+1;
        }
		//addItem changes currentIndex -> reset
		bodyUI->outDevCombobox->setCurrentIndex(-1);
		bodyUI->outDevCombobox->blockSignals(false);
    }
	bodyUI->outDevCombobox->setCurrentIndex(outDevIndex);
}

void AVForm::onInDevChanged(const QString &deviceDescriptor)
{
    Settings::getInstance().setInDev(deviceDescriptor);
    Audio::openInput(deviceDescriptor);
}

void AVForm::onOutDevChanged(const QString& deviceDescriptor)
{
    Settings::getInstance().setOutDev(deviceDescriptor);
    Audio::openOutput(deviceDescriptor);
}

void AVForm::onFilterAudioToggled(bool filterAudio)
{
    Settings::getInstance().setFilterAudio(filterAudio);
}

void AVForm::on_HueSlider_valueChanged(int value)
{
    Camera::getInstance()->setProp(Camera::HUE, value / 100.0);
    bodyUI->hueMax->setText(QString::number(value));
}

void AVForm::on_BrightnessSlider_valueChanged(int value)
{
    Camera::getInstance()->setProp(Camera::BRIGHTNESS, value / 100.0);
    bodyUI->brightnessMax->setText(QString::number(value));
}

void AVForm::on_SaturationSlider_valueChanged(int value)
{
    Camera::getInstance()->setProp(Camera::SATURATION, value / 100.0);
    bodyUI->saturationMax->setText(QString::number(value));
}

void AVForm::on_ContrastSlider_valueChanged(int value)
{
    Camera::getInstance()->setProp(Camera::CONTRAST, value / 100.0);
    bodyUI->contrastMax->setText(QString::number(value));
}

void AVForm::on_playbackSlider_valueChanged(int value)
{
    Audio::getInstance().outputVolume = value / 100.0;
    bodyUI->playbackMax->setText(QString::number(value));
}

void AVForm::on_microphoneSlider_valueChanged(int value)
{
    Audio::getInstance().outputVolume = value / 100.0;
    bodyUI->microphoneMax->setText(QString::number(value));
}

bool AVForm::eventFilter(QObject *o, QEvent *e)
{
    if ((e->type() == QEvent::Wheel) &&
         (qobject_cast<QComboBox*>(o) || qobject_cast<QAbstractSpinBox*>(o) ))
    {
        e->ignore();
        return true;
    }
    return QWidget::eventFilter(o, e);
}

void AVForm::createVideoSurface()
{
    if (CamVideoSurface)
        return;
    CamVideoSurface = new VideoSurface(bodyUI->CamFrame);
    CamVideoSurface->setObjectName(QStringLiteral("CamVideoSurface"));
    CamVideoSurface->setMinimumSize(QSize(160, 120));
    bodyUI->gridLayout->addWidget(CamVideoSurface, 0, 0, 1, 1);
}

void AVForm::killVideoSurface()
{
    if (!CamVideoSurface)
        return;
    QLayoutItem *child;
    while ((child = bodyUI->gridLayout->takeAt(0)) != 0)
        delete child;
    
    delete CamVideoSurface;
    CamVideoSurface = nullptr;
}
