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

    auto qcbxIndexChangedStr = (void(QComboBox::*)(const QString&)) &QComboBox::currentIndexChanged;
    auto qcbxIndexChangedInt = (void(QComboBox::*)(int)) &QComboBox::currentIndexChanged;
    connect(bodyUI->inDevCombobox, qcbxIndexChangedStr, this, &AVForm::onInDevChanged);
    connect(bodyUI->outDevCombobox, qcbxIndexChangedStr, this, &AVForm::onOutDevChanged);
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
    if (index < 0 || index >= videoModes.size())
    {
        qWarning() << "Invalid mode index";
        return;
    }
    int devIndex = bodyUI->videoDevCombobox->currentIndex();
    if (devIndex < 0 || devIndex >= videoDeviceList.size())
    {
        qWarning() << "Invalid device index";
        return;
    }
    QString devName = videoDeviceList[devIndex].first;
    VideoMode mode = videoModes[index];

    Settings::getInstance().setCamVideoRes(mode.toRect());
    Settings::getInstance().setCamVideoFPS(mode.FPS);
    camera.open(devName, mode);
}

void AVForm::selectBestModes(QVector<VideoMode> &allVideoModes)
{
    // Identify the best resolutions available for the supposed XXXXp resolutions.
    std::map<int, VideoMode> idealModes;
    idealModes[120] = VideoMode(160, 120);
    idealModes[240] = VideoMode(460, 240);
    idealModes[360] = VideoMode(640, 360);
    idealModes[480] = VideoMode(854, 480);
    idealModes[720] = VideoMode(1280, 720);
    idealModes[1080] = VideoMode(1920, 1080);

    std::map<int, int> bestModeInds;
    for (int i = 0; i < allVideoModes.size(); ++i)
    {
        VideoMode mode = allVideoModes[i];
        QString pixelFormat = CameraDevice::getPixelFormatString(mode.pixel_format);
        qDebug("width: %d, height: %d, FPS: %f, pixel format: %s", mode.width, mode.height, mode.FPS, pixelFormat.toStdString().c_str());

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

            int index = bestModeInds[res];
            VideoMode best = allVideoModes[index];
            if (mode.norm(idealMode) < best.norm(idealMode))
            {
                bestModeInds[res] = i;
                continue;
            }

            if (mode.norm(idealMode) == best.norm(idealMode))
            {
                // prefer higher FPS and "better" pixel formats
                if (mode.FPS > best.FPS)
                {
                    bestModeInds[res] = i;
                    continue;
                }

                bool better = CameraDevice::betterPixelFormat(mode.pixel_format, best.pixel_format);
                if (mode.FPS == best.FPS && better)
                    bestModeInds[res] = i;
            }
        }
    }

    QVector<VideoMode> newVideoModes;
    for (auto it = bestModeInds.rbegin(); it != bestModeInds.rend(); ++it)
    {
        VideoMode mode = allVideoModes[it->second];
        auto result = std::find(newVideoModes.begin(), newVideoModes.end(), mode);
        if (result == newVideoModes.end())
            newVideoModes.push_back(mode);
    }
    allVideoModes = newVideoModes;
}

void AVForm::fillCameraModesComboBox()
{
    bool previouslyBlocked = bodyUI->videoModescomboBox->blockSignals(true);
    bodyUI->videoModescomboBox->clear();

    for(int i = 0; i < videoModes.size(); i++)
    {
        VideoMode mode = videoModes[i];

        QString str;
        QString pixelFormat = CameraDevice::getPixelFormatString(mode.pixel_format);
        qDebug("width: %d, height: %d, FPS: %f, pixel format: %s\n", mode.width, mode.height, mode.FPS, pixelFormat.toStdString().c_str());

        if (mode.height && mode.width)
            str += QString("%1p").arg(mode.height);
        else
            str += tr("Default resolution");

        bodyUI->videoModescomboBox->addItem(str);
    }

    if (videoModes.isEmpty())
        bodyUI->videoModescomboBox->addItem(tr("Default resolution"));

    bodyUI->videoModescomboBox->blockSignals(previouslyBlocked);
}

int AVForm::searchPreferredIndex()
{
    QRect prefRes = Settings::getInstance().getCamVideoRes();
    unsigned short prefFPS = Settings::getInstance().getCamVideoFPS();

    for (int i = 0; i < videoModes.size(); i++)
    {
        VideoMode mode = videoModes[i];
        if (mode.width == prefRes.width()
                && mode.height == prefRes.height()
                && mode.FPS == prefFPS)
            return i;
    }

    return -1;
}

void AVForm::fillScreenModesComboBox()
{
    bool previouslyBlocked = bodyUI->videoModescomboBox->blockSignals(true);
    bodyUI->videoModescomboBox->clear();

    for(int i = 0; i < videoModes.size(); i++)
    {
        VideoMode mode = videoModes[i];
        QString pixelFormat = CameraDevice::getPixelFormatString(mode.pixel_format);
        qDebug("%dx%d+%d,%d FPS: %f, pixel format: %s\n", mode.width, mode.height, mode.x, mode.y, mode.FPS, pixelFormat.toStdString().c_str());

        QString name = QString("Screen %1").arg(i + 1);
        bodyUI->videoModescomboBox->addItem(name);
    }

    bodyUI->videoModescomboBox->addItem(tr("Select region"));
    bodyUI->videoModescomboBox->blockSignals(previouslyBlocked);
}

void AVForm::updateVideoModes(int curIndex)
{
    if (curIndex < 0 || curIndex >= videoDeviceList.size())
    {
        qWarning() << "Invalid index";
        return;
    }
    QString devName = videoDeviceList[curIndex].first;
    QVector<VideoMode> allVideoModes = CameraDevice::getVideoModes(devName);

    qDebug("available Modes:");
    if (CameraDevice::isScreen(devName))
    {
        videoModes = allVideoModes;
        fillScreenModesComboBox();
    }
    else
    {
        selectBestModes(allVideoModes);
        videoModes = allVideoModes;

        qDebug("selected Modes:");
        fillCameraModesComboBox();
    }

    int preferedIndex = searchPreferredIndex();
    if (preferedIndex!= -1)
    {
        bodyUI->videoModescomboBox->setCurrentIndex(preferedIndex);
        return;
    }

    // If the user hasn't set a preferred resolution yet,
    // we'll pick the resolution in the middle of the list,
    // and the best FPS for that resolution.
    // If we picked the lowest resolution, the quality would be awful
    // but if we picked the largest, FPS would be bad and thus quality bad too.
    int mid = videoModes.size() / 2;
    bodyUI->videoModescomboBox->setCurrentIndex(mid);
}

void AVForm::onVideoDevChanged(int index)
{
    if (index < 0 || index >= videoDeviceList.size())
    {
        qWarning() << "Invalid index";
        return;
    }

    QString dev = videoDeviceList[index].first;
    Settings::getInstance().setVideoDev(dev);
    bool previouslyBlocked = bodyUI->videoModescomboBox->blockSignals(true);
    updateVideoModes(index);
    bodyUI->videoModescomboBox->blockSignals(previouslyBlocked);

    int modeIndex = bodyUI->videoModescomboBox->currentIndex();
    VideoMode mode = VideoMode();
    if (0 < modeIndex || modeIndex < videoModes.size())
        mode = videoModes[modeIndex];

    camera.open(dev, mode);
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
    QString settingsInDev = Settings::getInstance().getInDev();
    int inDevIndex = 0;
    bodyUI->inDevCombobox->blockSignals(true);
    bodyUI->inDevCombobox->clear();
    bodyUI->inDevCombobox->addItem(tr("None"));
    const char* pDeviceList = Audio::inDeviceNames();
    if (pDeviceList)
    {
        while (*pDeviceList)
        {
            int len = strlen(pDeviceList);
            QString inDev = QString::fromUtf8(pDeviceList, len);
            bodyUI->inDevCombobox->addItem(inDev);
            if (settingsInDev == inDev)
                inDevIndex = bodyUI->inDevCombobox->count()-1;
            pDeviceList += len+1;
        }
    }
    bodyUI->inDevCombobox->blockSignals(false);
    bodyUI->inDevCombobox->setCurrentIndex(inDevIndex);
}

void AVForm::getAudioOutDevices()
{
    QString settingsOutDev = Settings::getInstance().getOutDev();
    int outDevIndex = 0;
    bodyUI->outDevCombobox->blockSignals(true);
    bodyUI->outDevCombobox->clear();
    bodyUI->outDevCombobox->addItem(tr("None"));
    const char* pDeviceList = Audio::outDeviceNames();
    if (pDeviceList)
    {
        while (*pDeviceList)
        {
            int len = strlen(pDeviceList);
            QString outDev = QString::fromUtf8(pDeviceList, len);
            bodyUI->outDevCombobox->addItem(outDev);
            if (settingsOutDev == outDev)
            {
                outDevIndex = bodyUI->outDevCombobox->count()-1;
            }
            pDeviceList += len+1;
        }
    }
    bodyUI->outDevCombobox->blockSignals(false);
    bodyUI->outDevCombobox->setCurrentIndex(outDevIndex);
}

void AVForm::onInDevChanged(QString deviceDescriptor)
{
    if (!bodyUI->inDevCombobox->currentIndex())
        deviceDescriptor = "none";

    Settings::getInstance().setInDev(deviceDescriptor);
    Audio& audio = Audio::getInstance();
    audio.reinitInput(deviceDescriptor);
    bodyUI->microphoneSlider->setEnabled(bodyUI->inDevCombobox->currentIndex() != 0);
    bodyUI->microphoneSlider->setSliderPosition(qRound(audio.inputGain() * 10.0));
}

void AVForm::onOutDevChanged(QString deviceDescriptor)
{
    if (!bodyUI->outDevCombobox->currentIndex())
        deviceDescriptor = "none";

    Settings::getInstance().setOutDev(deviceDescriptor);
    Audio& audio = Audio::getInstance();
    audio.reinitOutput(deviceDescriptor);
    bodyUI->playbackSlider->setEnabled(audio.isOutputReady());
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
