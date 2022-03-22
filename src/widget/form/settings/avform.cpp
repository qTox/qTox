/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#include <cassert>
#include <map>

#include <QDebug>
#include <QDesktopWidget>
#include <QScreen>
#include <QShowEvent>

#include "audio/audio.h"
#include "audio/iaudiosettings.h"
#include "audio/iaudiosource.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/video/cameradevice.h"
#include "src/video/camerasource.h"
#include "src/video/ivideosettings.h"
#include "src/video/videosurface.h"
#include "src/widget/tool/recursivesignalblocker.h"
#include "src/widget/tool/screenshotgrabber.h"
#include "src/widget/translator.h"

#ifndef ALC_ALL_DEVICES_SPECIFIER
#define ALC_ALL_DEVICES_SPECIFIER ALC_DEVICE_SPECIFIER
#endif

AVForm::AVForm(IAudioControl& audio_, CoreAV* coreAV_, CameraSource& camera_,
               IAudioSettings* audioSettings_, IVideoSettings* videoSettings_,
               Style& style)
    : GenericForm(QPixmap(":/img/settings/av.png"), style)
    , audio(audio_)
    , coreAV{coreAV_}
    , audioSettings{audioSettings_}
    , videoSettings{videoSettings_}
    , camVideoSurface(nullptr)
    , camera(camera_)
{
    setupUi(this);

    // block all child signals during initialization
    const RecursiveSignalBlocker signalBlocker(this);

    cbEnableTestSound->setChecked(audioSettings_->getEnableTestSound());
    cbEnableTestSound->setToolTip(tr("Play a test sound while changing the output volume."));

    connect(rescanButton, &QPushButton::clicked, this, &AVForm::rescanDevices);

    playbackSlider->setTracking(false);
    playbackSlider->setMaximum(totalSliderSteps);
    playbackSlider->setValue(getStepsFromValue(audioSettings_->getOutVolume(),
                                               audioSettings_->getOutVolumeMin(),
                                               audioSettings_->getOutVolumeMax()));
    playbackSlider->installEventFilter(this);

    microphoneSlider->setToolTip(tr("Use slider to set the gain of your input device ranging"
                                    " from %1dB to %2dB.")
                                     .arg(audio_.minInputGain())
                                     .arg(audio_.maxInputGain()));
    microphoneSlider->setMaximum(totalSliderSteps);
    microphoneSlider->setTickPosition(QSlider::TicksBothSides);
    static const int numTicks = 4;
    microphoneSlider->setTickInterval(totalSliderSteps / numTicks);
    microphoneSlider->setTracking(false);
    microphoneSlider->installEventFilter(this);
    microphoneSlider->setValue(
        getStepsFromValue(audioSettings_->getAudioInGainDecibel(),
            audio_.minInputGain(),
            audio_.maxInputGain()));

    audioThresholdSlider->setToolTip(tr("Use slider to set the activation volume for your"
                                        " input device."));
    audioThresholdSlider->setMaximum(totalSliderSteps);
    audioThresholdSlider->setValue(getStepsFromValue(audioSettings_->getAudioThreshold(),
                                                     audio_.minInputThreshold(),
                                                     audio_.maxInputThreshold()));
    audioThresholdSlider->setTracking(false);
    audioThresholdSlider->installEventFilter(this);

    volumeDisplay->setMaximum(totalSliderSteps);

    fillAudioQualityComboBox();

    eventsInit();

    for (QScreen* qScreen : QGuiApplication::screens()) {
        connect(qScreen, &QScreen::geometryChanged, this, &AVForm::rescanDevices);
    }
    auto* qGUIApp = qobject_cast<QGuiApplication *>(qApp);
    assert (qGUIApp);
    connect(qGUIApp, &QGuiApplication::screenAdded, this, &AVForm::trackNewScreenGeometry);
    connect(qGUIApp, &QGuiApplication::screenAdded, this, &AVForm::rescanDevices);
    connect(qGUIApp, &QGuiApplication::screenRemoved, this, &AVForm::rescanDevices);
    Translator::registerHandler(std::bind(&AVForm::retranslateUi, this), this);
}

AVForm::~AVForm()
{
    killVideoSurface();
    Translator::unregister(this);
}

void AVForm::hideEvent(QHideEvent* event)
{
    audioSink.reset();
    audioSrc.reset();

    if (camVideoSurface) {
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

    if (audioSrc == nullptr) {
        audioSrc = audio.makeSource();
        connect(audioSrc.get(), &IAudioSource::volumeAvailable, this, &AVForm::setVolume);
    }

    if (audioSink == nullptr) {
        audioSink = audio.makeSink();
    }

    GenericForm::showEvent(event);
}

void AVForm::open(const QString& devName, const VideoMode& mode)
{
    QRect rect = mode.toRect();
    videoSettings->setCamVideoRes(rect);
    videoSettings->setCamVideoFPS(static_cast<float>(mode.FPS));
    camera.setupDevice(devName, mode);
}

void AVForm::trackNewScreenGeometry(QScreen* qScreen) {
    connect(qScreen, &QScreen::geometryChanged, this, &AVForm::rescanDevices);
}

void AVForm::rescanDevices()
{
    getAudioInDevices();
    getAudioOutDevices();
    getVideoDevices();
}

void AVForm::setVolume(qreal value)
{
    volumeDisplay->setValue(getStepsFromValue(value, audio.minOutputVolume(), audio.maxOutputVolume()));
}

void AVForm::on_videoModescomboBox_currentIndexChanged(int index)
{
    assert(0 <= index && index < videoModes.size());
    int devIndex = videoDevCombobox->currentIndex();
    assert(0 <= devIndex && devIndex < videoDeviceList.size());

    QString devName = videoDeviceList[devIndex].first;
    VideoMode newMode = videoModes[index];

    if (CameraDevice::isScreen(devName) && newMode == VideoMode()) {
        if (videoSettings->getScreenGrabbed()) {
            VideoMode screenMode(videoSettings->getScreenRegion());
            open(devName, screenMode);
            return;
        }

        auto onGrabbed = [devName, this](QRect region) {
            VideoMode mode(region);
            mode.width = mode.width / 2 * 2;
            mode.height = mode.height / 2 * 2;

            // Needed, if the virtual screen origin is the top left corner of the primary screen
            QRect screen = QApplication::primaryScreen()->virtualGeometry();
            mode.x += screen.x();
            mode.y += screen.y();

            videoSettings->setScreenRegion(mode.toRect());
            videoSettings->setScreenGrabbed(true);

            open(devName, mode);
        };

        // note: grabber is self-managed and will destroy itself when done
        ScreenshotGrabber* screenshotGrabber = new ScreenshotGrabber;

        connect(screenshotGrabber, &ScreenshotGrabber::regionChosen, this, onGrabbed,
                Qt::QueuedConnection);
        screenshotGrabber->showGrabber();
        return;
    }

    videoSettings->setScreenGrabbed(false);
    open(devName, newMode);
}

void AVForm::selectBestModes(QVector<VideoMode>& allVideoModes)
{
    if (allVideoModes.isEmpty()) {
        qCritical() << "Trying to select best mode from empty modes list";
        return;
    }

    // Identify the best resolutions available for the supposed XXXXp resolutions.
    std::map<int, VideoMode> idealModes;
    idealModes[120] = VideoMode(160, 120);
    idealModes[240] = VideoMode(430, 240);
    idealModes[360] = VideoMode(640, 360);
    idealModes[480] = VideoMode(854, 480);
    idealModes[720] = VideoMode(1280, 720);
    idealModes[1080] = VideoMode(1920, 1080);
    idealModes[1440] = VideoMode(2560, 1440);
    idealModes[2160] = VideoMode(3840, 2160);

    std::map<int, int> bestModeInds;
    for (int i = 0; i < allVideoModes.size(); ++i) {
        VideoMode mode = allVideoModes[i];

        // PS3-Cam protection, everything above 60fps makes no sense
        if (mode.FPS > 60)
            continue;

        for (auto iter = idealModes.begin(); iter != idealModes.end(); ++iter) {
            int res = iter->first;
            VideoMode idealMode = iter->second;
            // don't take approximately correct resolutions unless they really
            // are close
            if (mode.norm(idealMode) > idealMode.tolerance())
                continue;

            if (bestModeInds.find(res) == bestModeInds.end()) {
                bestModeInds[res] = i;
                continue;
            }

            int index = bestModeInds[res];
            VideoMode best = allVideoModes[index];
            if (mode.norm(idealMode) < best.norm(idealMode)) {
                bestModeInds[res] = i;
                continue;
            }

            if (mode.norm(idealMode) == best.norm(idealMode)) {
                // prefer higher FPS and "better" pixel formats
                if (mode.FPS > best.FPS) {
                    bestModeInds[res] = i;
                    continue;
                }

                bool better = CameraDevice::betterPixelFormat(mode.pixel_format, best.pixel_format);
                if (mode.FPS >= best.FPS && better)
                    bestModeInds[res] = i;
            }
        }
    }

    QVector<VideoMode> newVideoModes;
    for (auto it = bestModeInds.rbegin(); it != bestModeInds.rend(); ++it) {
        VideoMode mode_ = allVideoModes[it->second];

        if (newVideoModes.empty()) {
            newVideoModes.push_back(mode_);
        } else {
            int size = getModeSize(mode_);
            auto result = std::find_if(newVideoModes.cbegin(), newVideoModes.cend(),
                                       [size](VideoMode mode) { return getModeSize(mode) == size; });

            if (result == newVideoModes.end())
                newVideoModes.push_back(mode_);
        }
    }
    allVideoModes = newVideoModes;
}

void AVForm::fillCameraModesComboBox()
{
    qDebug() << "selected Modes:";
    bool previouslyBlocked = videoModescomboBox->blockSignals(true);
    videoModescomboBox->clear();

    for (int i = 0; i < videoModes.size(); ++i) {
        VideoMode mode = videoModes[i];

        QString str;
        std::string pixelFormat = CameraDevice::getPixelFormatString(mode.pixel_format).toStdString();
        qDebug("width: %d, height: %d, FPS: %f, pixel format: %s", mode.width, mode.height,
               static_cast<double>(mode.FPS), pixelFormat.c_str());

        if (mode.height && mode.width) {
            str += QString("%1p").arg(mode.height);
        } else {
            str += tr("Default resolution");
        }

        videoModescomboBox->addItem(str);
    }

    if (videoModes.isEmpty())
        videoModescomboBox->addItem(tr("Default resolution"));

    videoModescomboBox->blockSignals(previouslyBlocked);
}

int AVForm::searchPreferredIndex()
{
    QRect prefRes = videoSettings->getCamVideoRes();
    float prefFPS = videoSettings->getCamVideoFPS();

    for (int i = 0; i < videoModes.size(); ++i) {
        VideoMode mode = videoModes[i];
        if (mode.width == prefRes.width() && mode.height == prefRes.height()
            && (qAbs(mode.FPS - prefFPS) < 0.0001f)) {
            return i;
        }
    }

    return -1;
}

void AVForm::fillScreenModesComboBox()
{
    bool previouslyBlocked = videoModescomboBox->blockSignals(true);
    videoModescomboBox->clear();

    for (int i = 0; i < videoModes.size(); ++i) {
        VideoMode mode = videoModes[i];
        std::string pixelFormat = CameraDevice::getPixelFormatString(mode.pixel_format).toStdString();
        qDebug("%dx%d+%d,%d FPS: %f, pixel format: %s\n", mode.width, mode.height, mode.x, mode.y,
               static_cast<double>(mode.FPS), pixelFormat.c_str());

        QString name;
        if (mode.width && mode.height)
            name = tr("Screen %1").arg(i + 1);
        else
            name = tr("Select region");

        videoModescomboBox->addItem(name);
    }

    videoModescomboBox->blockSignals(previouslyBlocked);
}

void AVForm::fillAudioQualityComboBox()
{
    const bool previouslyBlocked = audioQualityComboBox->blockSignals(true);

    audioQualityComboBox->addItem(tr("High (64 kbps)"), 64);
    audioQualityComboBox->addItem(tr("Medium (32 kbps)"), 32);
    audioQualityComboBox->addItem(tr("Low (16 kbps)"), 16);
    audioQualityComboBox->addItem(tr("Very low (8 kbps)"), 8);

    const int currentBitrate = audioSettings->getAudioBitrate();
    const int index = audioQualityComboBox->findData(currentBitrate);

    audioQualityComboBox->setCurrentIndex(index);
    audioQualityComboBox->blockSignals(previouslyBlocked);
}

void AVForm::updateVideoModes(int curIndex)
{
    if (curIndex < 0 || curIndex >= videoDeviceList.size()) {
        qWarning() << "Invalid index:" << curIndex;
        return;
    }
    QString devName = videoDeviceList[curIndex].first;
    QVector<VideoMode> allVideoModes = CameraDevice::getVideoModes(devName);

    qDebug("available Modes:");
    bool isScreen = CameraDevice::isScreen(devName);
    if (isScreen) {
        // Add extra video mode to region selection
        allVideoModes.push_back(VideoMode());
        videoModes = allVideoModes;
        fillScreenModesComboBox();
    } else {
        selectBestModes(allVideoModes);
        videoModes = allVideoModes;
        fillCameraModesComboBox();
    }

    int preferedIndex = searchPreferredIndex();
    if (preferedIndex != -1) {
        videoSettings->setScreenGrabbed(false);
        videoModescomboBox->blockSignals(true);
        videoModescomboBox->setCurrentIndex(preferedIndex);
        videoModescomboBox->blockSignals(false);
        open(devName, videoModes[preferedIndex]);
        return;
    }

    if (isScreen) {
        QRect rect = videoSettings->getScreenRegion();
        VideoMode mode(rect);

        videoSettings->setScreenGrabbed(true);
        videoModescomboBox->setCurrentIndex(videoModes.size() - 1);
        open(devName, mode);
        return;
    }

    // If the user hasn't set a preferred resolution yet,
    // we'll pick the resolution in the middle of the list,
    // and the best FPS for that resolution.
    // If we picked the lowest resolution, the quality would be awful
    // but if we picked the largest, FPS would be bad and thus quality bad too.
    int mid = (videoModes.size() - 1) / 2;
    videoModescomboBox->setCurrentIndex(mid);
}

void AVForm::on_videoDevCombobox_currentIndexChanged(int index)
{
    assert(0 <= index && index < videoDeviceList.size());

    videoSettings->setScreenGrabbed(false);
    QString dev = videoDeviceList[index].first;
    videoSettings->setVideoDev(dev);
    bool previouslyBlocked = videoModescomboBox->blockSignals(true);
    updateVideoModes(index);
    videoModescomboBox->blockSignals(previouslyBlocked);

    if (videoSettings->getScreenGrabbed()) {
        return;
    }

    int modeIndex = videoModescomboBox->currentIndex();
    VideoMode mode = VideoMode();
    if (0 <= modeIndex && modeIndex < videoModes.size()) {
        mode = videoModes[modeIndex];
    }

    camera.setupDevice(dev, mode);
    if (dev == "none") {
        coreAV->sendNoVideo();
    }
}

void AVForm::on_audioQualityComboBox_currentIndexChanged(int index)
{
    std::ignore = index;
    audioSettings->setAudioBitrate(audioQualityComboBox->currentData().toInt());
}

void AVForm::getVideoDevices()
{
    QString settingsInDev = videoSettings->getVideoDev();
    int videoDevIndex = 0;
    videoDeviceList = CameraDevice::getDeviceList();
    // prevent currentIndexChanged to be fired while adding items
    videoDevCombobox->blockSignals(true);
    videoDevCombobox->clear();
    for (QPair<QString, QString> device : videoDeviceList) {
        videoDevCombobox->addItem(device.second);
        if (device.first == settingsInDev)
            videoDevIndex = videoDevCombobox->count() - 1;
    }
    videoDevCombobox->setCurrentIndex(videoDevIndex);
    videoDevCombobox->blockSignals(false);
    updateVideoModes(videoDevIndex);
}

int AVForm::getModeSize(VideoMode mode)
{
    return qRound(mode.height / 120.0) * 120;
}

void AVForm::getAudioInDevices()
{
    QStringList deviceNames;
    deviceNames << tr("Disabled") << audio.inDeviceNames();

    inDevCombobox->blockSignals(true);
    inDevCombobox->clear();
    inDevCombobox->addItems(deviceNames);
    inDevCombobox->blockSignals(false);

    int idx = 0;
    bool enabled = audioSettings->getAudioInDevEnabled();
    if (enabled && deviceNames.size() > 1) {
        QString dev = audioSettings->getInDev();
        idx = qMax(deviceNames.indexOf(dev), 1);
    }
    inDevCombobox->setCurrentIndex(idx);
}

void AVForm::getAudioOutDevices()
{
    QStringList deviceNames;
    deviceNames << tr("Disabled") << audio.outDeviceNames();

    outDevCombobox->blockSignals(true);
    outDevCombobox->clear();
    outDevCombobox->addItems(deviceNames);
    outDevCombobox->blockSignals(false);

    int idx = 0;
    bool enabled = audioSettings->getAudioOutDevEnabled();
    if (enabled && deviceNames.size() > 1) {
        QString dev = audioSettings->getOutDev();
        idx = qMax(deviceNames.indexOf(dev), 1);
    }
    outDevCombobox->setCurrentIndex(idx);
}

void AVForm::on_inDevCombobox_currentIndexChanged(int deviceIndex)
{
    const bool inputEnabled = deviceIndex > 0;
    audioSettings->setAudioInDevEnabled(inputEnabled);

    QString deviceName{};
    if (inputEnabled) {
        deviceName = inDevCombobox->itemText(deviceIndex);
    }

    const QString oldName = audioSettings->getInDev();
    if (oldName != deviceName) {
        audioSettings->setInDev(deviceName);
        audio.reinitInput(deviceName);
        audioSrc = audio.makeSource();
        connect(audioSrc.get(), &IAudioSource::volumeAvailable, this, &AVForm::setVolume);
    }

    microphoneSlider->setEnabled(inputEnabled);
    if (!inputEnabled) {
        volumeDisplay->setValue(volumeDisplay->minimum());
    }
}

void AVForm::on_outDevCombobox_currentIndexChanged(int deviceIndex)
{
    const bool outputEnabled = deviceIndex > 0;
    audioSettings->setAudioOutDevEnabled(outputEnabled);

    QString deviceName{};
    if (outputEnabled) {
        deviceName = outDevCombobox->itemText(deviceIndex);
    }

    const QString oldName = audioSettings->getOutDev();

    if (oldName != deviceName) {
        audioSettings->setOutDev(deviceName);
        audio.reinitOutput(deviceName);
        audioSink = audio.makeSink();
    }

    playbackSlider->setEnabled(outputEnabled);
}

void AVForm::on_playbackSlider_valueChanged(int sliderSteps)
{
    const int settingsVolume = getValueFromSteps(sliderSteps, audioSettings->getOutVolumeMin(),
                                                 audioSettings->getOutVolumeMax());
    audioSettings->setOutVolume(settingsVolume);

    if (audio.isOutputReady()) {
        const qreal volume =
            getValueFromSteps(sliderSteps, audio.minOutputVolume(), audio.maxOutputVolume());
        audio.setOutputVolume(volume);

        if (cbEnableTestSound->isChecked() && audioSink) {
            audioSink->playMono16Sound(IAudioSink::Sound::Test);
        }
    }
}

void AVForm::on_cbEnableTestSound_stateChanged()
{
    audioSettings->setEnableTestSound(cbEnableTestSound->isChecked());

    if (cbEnableTestSound->isChecked() && audio.isOutputReady() && audioSink) {
        audioSink->playMono16Sound(IAudioSink::Sound::Test);
    }
}

void AVForm::on_microphoneSlider_valueChanged(int sliderSteps)
{
    const qreal dB = getValueFromSteps(sliderSteps, audio.minInputGain(), audio.maxInputGain());
    audioSettings->setAudioInGainDecibel(dB);
    audio.setInputGain(dB);
}

void AVForm::on_audioThresholdSlider_valueChanged(int sliderSteps)
{
    const qreal normThreshold =
        getValueFromSteps(sliderSteps, audio.minInputThreshold(), audio.maxInputThreshold());
    audioSettings->setAudioThreshold(normThreshold);
    audio.setInputThreshold(normThreshold);
}
void AVForm::createVideoSurface()
{
    if (camVideoSurface)
        return;

    camVideoSurface = new VideoSurface(QPixmap(), CamFrame);
    camVideoSurface->setObjectName(QStringLiteral("CamVideoSurface"));
    camVideoSurface->setMinimumSize(QSize(160, 120));
    camVideoSurface->setSource(&camera);
    gridLayout->addWidget(camVideoSurface, 0, 0, 1, 1);
}

void AVForm::killVideoSurface()
{
    if (!camVideoSurface)
        return;

    QLayoutItem* child;
    while ((child = gridLayout->takeAt(0)) != nullptr)
        delete child;

    camVideoSurface->close();
    delete camVideoSurface;
    camVideoSurface = nullptr;
}

void AVForm::retranslateUi()
{
    Ui::AVForm::retranslateUi(this);
}

int AVForm::getStepsFromValue(qreal val, qreal valMin, qreal valMax)
{
    const float norm = (val - valMin) / (valMax - valMin);
    return norm * totalSliderSteps;
}

qreal AVForm::getValueFromSteps(int steps, qreal valMin, qreal valMax)
{
    return (static_cast<qreal>(steps) / totalSliderSteps) * (valMax - valMin) + valMin;
}
