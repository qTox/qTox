/*
    Copyright © 2014-2015 by The qTox Project

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "avform.h"
#include "ui_avsettings.h"
#include "src/persistence/settings.h"
#include "src/audio/audio.h"
#include "src/video/camerasource.h"
#include "src/video/cameradevice.h"
#include "src/video/videosurface.h"
#include "src/widget/translator.h"

#if defined(__APPLE__) && defined(__MACH__)
 #include <OpenAL/al.h>
 #include <OpenAL/alc.h>
#else
 #include <AL/alc.h>
 #include <AL/al.h>
#endif

#include <QDebug>

#ifndef ALC_ALL_DEVICES_SPECIFIER
#define ALC_ALL_DEVICES_SPECIFIER ALC_DEVICE_SPECIFIER
#endif

AVForm::AVForm() :
    GenericForm(QPixmap(":/img/settings/av.png")),
    camVideoSurface{nullptr}, camera{CameraSource::getInstance()}
{
    bodyUI = new Ui::AVSettings;
    bodyUI->setupUi(this);

#ifdef QTOX_FILTER_AUDIO
    bodyUI->filterAudio->setChecked(Settings::getInstance().getFilterAudio());
#else
    bodyUI->filterAudio->setDisabled(true);
#endif

    auto qcbxIndexChangedStr = (void(QComboBox::*)(const QString&)) &QComboBox::currentIndexChanged;
    auto qcbxIndexChangedInt = (void(QComboBox::*)(int)) &QComboBox::currentIndexChanged;
    connect(bodyUI->inDevCombobox, qcbxIndexChangedStr, this, &AVForm::onInDevChanged);
    connect(bodyUI->outDevCombobox, qcbxIndexChangedStr, this, &AVForm::onOutDevChanged);
    connect(bodyUI->videoDevCombobox, qcbxIndexChangedInt, this, &AVForm::onVideoDevChanged);
    connect(bodyUI->filterAudio, &QCheckBox::toggled, this, &AVForm::onFilterAudioToggled);
    connect(bodyUI->rescanButton, &QPushButton::clicked, this, [=](){getAudioInDevices(); getAudioOutDevices();});
    bodyUI->playbackSlider->setValue(100);
    bodyUI->microphoneSlider->setValue(100);

    for (QComboBox* cb : findChildren<QComboBox*>())
    {
        cb->installEventFilter(this);
        cb->setFocusPolicy(Qt::StrongFocus);
    }

    Translator::registerHandler(std::bind(&AVForm::retranslateUi, this), this);
}

AVForm::~AVForm()
{
    Translator::unregister(this);
    delete bodyUI;
}

void AVForm::showEvent(QShowEvent*)
{
    getAudioOutDevices();
    getAudioInDevices();
    createVideoSurface();
    getVideoDevices();
}

void AVForm::on_videoModescomboBox_currentIndexChanged(int index)
{
    if (index<0 || index>=videoModes.size())
    {
        qWarning() << "Invalid mode index";
        return;
    }
    int devIndex = bodyUI->videoDevCombobox->currentIndex();
    if (devIndex<0 || devIndex>=videoModes.size())
    {
        qWarning() << "Invalid device index";
        return;
    }
    QString devName = videoDeviceList[devIndex].first;
    VideoMode mode = videoModes[index];
    Settings::getInstance().setCamVideoRes(QSize(mode.width, mode.height));
    camera.open(devName, mode);
}

void AVForm::updateVideoModes(int curIndex)
{
    if (curIndex<0 || curIndex>=videoDeviceList.size())
    {
        qWarning() << "Invalid index";
        return;
    }
    QString devName = videoDeviceList[curIndex].first;
    videoModes = CameraDevice::getVideoModes(devName);
    std::sort(videoModes.begin(), videoModes.end(),
        [](const VideoMode& a, const VideoMode& b)
            {return a.width!=b.width ? a.width>b.width :
                    a.height!=b.height ? a.height>b.height :
                    a.FPS>b.FPS;});
    bool previouslyBlocked = bodyUI->videoModescomboBox->blockSignals(true);
    bodyUI->videoModescomboBox->clear();
    int prefResIndex = -1;
    QSize prefRes = Settings::getInstance().getCamVideoRes();
    for (int i=0; i<videoModes.size(); ++i)
    {
        VideoMode mode = videoModes[i];
        if (mode.width==prefRes.width() && mode.height==prefRes.height() && prefResIndex==-1)
            prefResIndex = i;
        QString str;
        if (mode.height && mode.width)
            str += tr("%1x%2").arg(mode.width).arg(mode.height);
        else
            str += tr("Default resolution");
        if (mode.FPS)
            str += tr(" at %1 FPS").arg(mode.FPS);
        bodyUI->videoModescomboBox->addItem(str);
    }
    if (videoModes.isEmpty())
        bodyUI->videoModescomboBox->addItem(tr("Default resolution"));
    bodyUI->videoModescomboBox->blockSignals(previouslyBlocked);
    if (prefResIndex != -1)
    {
        bodyUI->videoModescomboBox->setCurrentIndex(prefResIndex);
    }
    else
    {
        // If the user hasn't set a preffered resolution yet,
        // we'll pick the resolution in the middle of the list,
        // and the best FPS for that resolution.
        // If we picked the lowest resolution, the quality would be awful
        // but if we picked the largest, FPS would be bad and thus quality bad too.
        int numRes=0;
        QSize lastSize;
        for (int i=0; i<videoModes.size(); i++)
        {
            if (lastSize != QSize{videoModes[i].width, videoModes[i].height})
            {
                numRes++;
                lastSize = {videoModes[i].width, videoModes[i].height};
            }
        }
        int target = numRes/2;
        numRes=0;
        for (int i=0; i<videoModes.size(); i++)
        {
            if (lastSize != QSize{videoModes[i].width, videoModes[i].height})
            {
                numRes++;
                lastSize = {videoModes[i].width, videoModes[i].height};
            }
            if (numRes==target)
            {
                bodyUI->videoModescomboBox->setCurrentIndex(i);
                break;
            }
        }
    }


}

void AVForm::onVideoDevChanged(int index)
{
    if (index<0 || index>=videoDeviceList.size())
    {
        qWarning() << "Invalid index";
        return;
    }
    QString dev = videoDeviceList[index].first;
    Settings::getInstance().setVideoDev(dev);
    bool previouslyBlocked = bodyUI->videoModescomboBox->blockSignals(true);
    updateVideoModes(index);
    bodyUI->videoModescomboBox->blockSignals(previouslyBlocked);
    camera.open(dev);
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
    if (camVideoSurface)
    {
        camVideoSurface->setSource(nullptr);
        killVideoSurface();
    }
    videoDeviceList.clear();
}

void AVForm::getVideoDevices()
{
    QString settingsInDev = Settings::getInstance().getVideoDev();
    int videoDevIndex = 0;
    videoDeviceList = CameraDevice::getDeviceList();
    //prevent currentIndexChanged to be fired while adding items
    bodyUI->videoDevCombobox->blockSignals(true);
    bodyUI->videoDevCombobox->clear();
    for (QPair<QString, QString> device : videoDeviceList)
    {
        bodyUI->videoDevCombobox->addItem(device.second);
        if (device.first == settingsInDev)
            videoDevIndex = bodyUI->videoDevCombobox->count()-1;
    }
    //addItem changes currentIndex -> reset
    bodyUI->videoDevCombobox->setCurrentIndex(-1);
    bodyUI->videoDevCombobox->setCurrentIndex(videoDevIndex);
    bodyUI->videoDevCombobox->blockSignals(false);
    updateVideoModes(videoDevIndex);
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
				inDevIndex = bodyUI->inDevCombobox->count()-1;
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

void AVForm::on_playbackSlider_valueChanged(int value)
{
    Audio::setOutputVolume(value / 100.0);
    bodyUI->playbackMax->setText(QString::number(value));
}

void AVForm::on_microphoneSlider_valueChanged(int value)
{
    Audio::setOutputVolume(value / 100.0);
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
    if (camVideoSurface)
        return;
    camVideoSurface = new VideoSurface(bodyUI->CamFrame);
    camVideoSurface->setObjectName(QStringLiteral("CamVideoSurface"));
    camVideoSurface->setMinimumSize(QSize(160, 120));
    camVideoSurface->setSource(&camera);
    bodyUI->gridLayout->addWidget(camVideoSurface, 0, 0, 1, 1);
}

void AVForm::killVideoSurface()
{
    if (!camVideoSurface)
        return;
    QLayoutItem *child;
    while ((child = bodyUI->gridLayout->takeAt(0)) != 0)
        delete child;

    delete camVideoSurface;
    camVideoSurface = nullptr;
}

void AVForm::retranslateUi()
{
    bodyUI->retranslateUi(this);
}
