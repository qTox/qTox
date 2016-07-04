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
#include "src/audio/audio.h"
#include "src/persistence/settings.h"
#include "src/video/camerasource.h"
#include "src/video/cameradevice.h"
#include "src/video/videosurface.h"
#include "src/widget/translator.h"
#include "src/core/core.h"
#include "src/core/coreav.h"

#include <QDebug>
#include <QShowEvent>
#include <map>

#ifndef ALC_ALL_DEVICES_SPECIFIER
#define ALC_ALL_DEVICES_SPECIFIER ALC_DEVICE_SPECIFIER
#endif

AVForm::AVForm() :
    GenericForm(QPixmap(":/img/settings/av.png"))
    , subscribedToAudioIn{false}
    , camVideoSurface{nullptr}
    , camera(CameraSource::getInstance())
{
    bodyUI = new Ui::AVSettings;
    bodyUI->setupUi(this);

    const Audio& audio = Audio::getInstance();

    bodyUI->btnPlayTestSound->setToolTip(
                tr("Play a test sound while changing the output volume."));

    auto qcbxIndexChangedInt = static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged);

    connect(bodyUI->inDevCombobox, qcbxIndexChangedInt, this, &AVForm::onAudioInDevChanged);
    connect(bodyUI->outDevCombobox, qcbxIndexChangedInt, this, &AVForm::onAudioOutDevChanged);
    connect(bodyUI->videoDevCombobox, qcbxIndexChangedInt, this, &AVForm::onVideoDevChanged);
    connect(bodyUI->videoModescomboBox, qcbxIndexChangedInt, this, &AVForm::onVideoModesIndexChanged);
    connect(bodyUI->rescanButton, &QPushButton::clicked, this, [=]()
    {
        getAudioInDevices();
        getAudioOutDevices();
        getVideoDevices();
    });

    bodyUI->playbackSlider->setTracking(false);
    bodyUI->playbackSlider->installEventFilter(this);
    connect(bodyUI->playbackSlider, &QSlider::valueChanged,
            this, &AVForm::onPlaybackValueChanged);

    bodyUI->microphoneSlider->setToolTip(
                tr("Use slider to set the gain of your input device ranging"
                   " from %1dB to %2dB.")
                .arg(audio.minInputGain())
                .arg(audio.maxInputGain()));
    bodyUI->microphoneSlider->setMinimum(qRound(audio.minInputGain()) * 10);
    bodyUI->microphoneSlider->setMaximum(qRound(audio.maxInputGain()) * 10);
    bodyUI->microphoneSlider->setTickPosition(QSlider::TicksBothSides);
    bodyUI->microphoneSlider->setTickInterval(
                (qAbs(bodyUI->microphoneSlider->minimum()) +
                 bodyUI->microphoneSlider->maximum()) / 4);
    bodyUI->microphoneSlider->setTracking(false);
    bodyUI->microphoneSlider->installEventFilter(this);
    connect(bodyUI->microphoneSlider, &QSlider::valueChanged,
            this, &AVForm::onMicrophoneValueChanged);

    for (QComboBox* cb : findChildren<QComboBox*>())
    {
        cb->installEventFilter(this);
        cb->setFocusPolicy(Qt::StrongFocus);
    }

    Translator::registerHandler(std::bind(&AVForm::retranslateUi, this), this);
}

AVForm::~AVForm()
{
    killVideoSurface();
    Translator::unregister(this);
    delete bodyUI;
}

void AVForm::hideEvent(QHideEvent* event)
{
    if (subscribedToAudioIn) {
        // TODO: this should not be done in show/hide events
        Audio::getInstance().unsubscribeInput();
        subscribedToAudioIn = false;
    }

    if (camVideoSurface)
    {
        camVideoSurface->setSource(nullptr);
        killVideoSurface();
    }
    videoDeviceList.clear();

    GenericForm::hideEvent(event);
}

void AVForm::showEvent(QShowEvent* event)
{
    getAudioOutDevices();
    getAudioInDevices();
    createVideoSurface();
    getVideoDevices();

    if (!subscribedToAudioIn) {
        // TODO: this should not be done in show/hide events
        Audio::getInstance().subscribeInput();
        subscribedToAudioIn = true;
    }

    GenericForm::showEvent(event);
}

void AVForm::onVideoModesIndexChanged(int index)
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
    Settings::getInstance().setCamVideoFPS(mode.FPS);
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
    QVector<VideoMode> allVideoModes = CameraDevice::getVideoModes(devName);
    std::sort(allVideoModes.begin(), allVideoModes.end(),
        [](const VideoMode& a, const VideoMode& b)
            {return a.width!=b.width ? a.width>b.width :
                    a.height!=b.height ? a.height>b.height :
                    a.FPS>b.FPS;});
    bool previouslyBlocked = bodyUI->videoModescomboBox->blockSignals(true);
    bodyUI->videoModescomboBox->clear();

    // Identify the best resolutions available for the supposed XXXXp resolutions.
    std::map<int, VideoMode> idealModes;
    idealModes[120] = {160,120,0,0};
    idealModes[240] = {460,240,0,0};
    idealModes[360] = {640,360,0,0};
    idealModes[480] = {854,480,0,0};
    idealModes[720] = {1280,720,0,0};
    idealModes[1080] = {1920,1080,0,0};
    std::map<int, int> bestModeInds;

    qDebug("available Modes:");
    for (int i=0; i<allVideoModes.size(); ++i)
    {
        VideoMode mode = allVideoModes[i];
        qDebug("width: %d, height: %d, FPS: %f, pixel format: %s", mode.width, mode.height, mode.FPS, CameraDevice::getPixelFormatString(mode.pixel_format).toStdString().c_str());

        // PS3-Cam protection, everything above 60fps makes no sense
        if(mode.FPS > 60)
            continue;

        for(auto iter = idealModes.begin(); iter != idealModes.end(); ++iter)
        {
            int res = iter->first;
            VideoMode idealMode = iter->second;
            // don't take approximately correct resolutions unless they really
            // are close
            if (mode.norm(idealMode) > 300)
                continue;

            if (bestModeInds.find(res) == bestModeInds.end())
            {
                bestModeInds[res] = i;
                continue;
            }
            int ind = bestModeInds[res];
            if (mode.norm(idealMode) < allVideoModes[ind].norm(idealMode))
            {
                bestModeInds[res] = i;
            }
            else if (mode.norm(idealMode) == allVideoModes[ind].norm(idealMode))
            {
                // prefer higher FPS and "better" pixel formats
                if (mode.FPS > allVideoModes[ind].FPS)
                {
                    bestModeInds[res] = i;
                }
                else if (mode.FPS == allVideoModes[ind].FPS &&
                        CameraDevice::betterPixelFormat(mode.pixel_format, allVideoModes[ind].pixel_format))
                {
                    bestModeInds[res] = i;
                }
            }
        }
    }
    qDebug("selected Modes:");
    int prefResIndex = -1;
    QSize prefRes = Settings::getInstance().getCamVideoRes();
    unsigned short prefFPS = Settings::getInstance().getCamVideoFPS();
    // Iterate backwards to show higest resolution first.
    videoModes.clear();
    for(auto iter = bestModeInds.rbegin(); iter != bestModeInds.rend(); ++iter)
    {
        int i = iter->second;
        VideoMode mode = allVideoModes[i];

        if (videoModes.contains(mode))
            continue;

        videoModes.append(mode);
        if (mode.width==prefRes.width() && mode.height==prefRes.height() && mode.FPS == prefFPS && prefResIndex==-1)
            prefResIndex = videoModes.size() - 1;
        QString str;
        qDebug("width: %d, height: %d, FPS: %f, pixel format: %s\n", mode.width, mode.height, mode.FPS, CameraDevice::getPixelFormatString(mode.pixel_format).toStdString().c_str());
        if (mode.height && mode.width)
            str += tr("%1p").arg(iter->first);
        else
            str += tr("Default resolution");
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

        if (videoModes.size())
        {
            bodyUI->videoModescomboBox->setUpdatesEnabled(false);
            bodyUI->videoModescomboBox->setCurrentIndex(-1);
            bodyUI->videoModescomboBox->setUpdatesEnabled(true);
            bodyUI->videoModescomboBox->setCurrentIndex(0);
        }
        else
        {
            // We don't have any video modes, open it with the default mode
            camera.open(devName);
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
    if (dev == "none")
        Core::getInstance()->getAv()->sendNoVideo();
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
    bodyUI->videoDevCombobox->setCurrentIndex(videoDevIndex);
    bodyUI->videoDevCombobox->blockSignals(false);
    updateVideoModes(videoDevIndex);
}

void AVForm::getAudioInDevices()
{
    QStringList deviceNames;
    deviceNames << tr("None") << Audio::inDeviceNames();

    bodyUI->inDevCombobox->blockSignals(true);
    bodyUI->inDevCombobox->clear();
    bodyUI->inDevCombobox->addItems(deviceNames);
    bodyUI->inDevCombobox->blockSignals(false);

    int idx = deviceNames.indexOf(Settings::getInstance().getInDev());
    bodyUI->inDevCombobox->setCurrentIndex(idx > 0 ? idx : 0);
}

void AVForm::getAudioOutDevices()
{
    QStringList deviceNames;
    deviceNames << tr("None") << Audio::outDeviceNames();

    bodyUI->outDevCombobox->blockSignals(true);
    bodyUI->outDevCombobox->clear();
    bodyUI->outDevCombobox->addItems(deviceNames);
    bodyUI->outDevCombobox->blockSignals(false);

    int idx = deviceNames.indexOf(Settings::getInstance().getOutDev());
    bodyUI->outDevCombobox->setCurrentIndex(idx > 0 ? idx : 0);
}

void AVForm::onAudioInDevChanged(int deviceIndex)
{
    QString deviceName = deviceIndex > 0
                         ? bodyUI->inDevCombobox->itemText(deviceIndex)
                         : QStringLiteral("none");

    Settings::getInstance().setInDev(deviceName);

    Audio& audio = Audio::getInstance();
    audio.reinitInput(deviceName);
    bodyUI->microphoneSlider->setEnabled(deviceIndex > 0);
    bodyUI->microphoneSlider->setSliderPosition(qRound(audio.inputGain() * 10.0));
}

void AVForm::onAudioOutDevChanged(int deviceIndex)
{
    QString deviceName = deviceIndex > 0
                         ? bodyUI->outDevCombobox->itemText(deviceIndex)
                         : QStringLiteral("none");

    Settings::getInstance().setOutDev(deviceName);

    Audio& audio = Audio::getInstance();
    audio.reinitOutput(deviceName);
    bodyUI->playbackSlider->setEnabled(deviceIndex > 0);
    bodyUI->playbackSlider->setSliderPosition(qRound(audio.outputVolume() * 100.0));
}

void AVForm::onPlaybackValueChanged(int value)
{
    Settings::getInstance().setOutVolume(value);

    Audio& audio = Audio::getInstance();
    if (audio.isOutputReady()) {
        const qreal percentage = value / 100.0;
        audio.setOutputVolume(percentage);

        if (mPlayTestSound)
            audio.playMono16Sound(QStringLiteral(":/audio/notification.pcm"));
    }
}

void AVForm::onMicrophoneValueChanged(int value)
{
    const qreal dB = value / 10.0;

    Settings::getInstance().setAudioInGain(dB);
    Audio::getInstance().setInputGain(dB);
}

void AVForm::createVideoSurface()
{
    if (camVideoSurface)
        return;
    camVideoSurface = new VideoSurface(QPixmap(), bodyUI->CamFrame);
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

    camVideoSurface->close();
    delete camVideoSurface;
    camVideoSurface = nullptr;
}

bool AVForm::eventFilter(QObject *o, QEvent *e)
{
    if ((e->type() == QEvent::Wheel) &&
         (qobject_cast<QComboBox*>(o) || qobject_cast<QAbstractSpinBox*>(o) || qobject_cast<QSlider*>(o)))
    {
        e->ignore();
        return true;
    }
    return QWidget::eventFilter(o, e);
}

void AVForm::retranslateUi()
{
    bodyUI->retranslateUi(this);
}

void AVForm::on_btnPlayTestSound_clicked(bool checked)
{
    mPlayTestSound = checked;
}
