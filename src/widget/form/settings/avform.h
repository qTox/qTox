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

#ifndef AVFORM_H
#define AVFORM_H

#include <QList>
#include <QObject>
#include <QString>

#include "genericsettings.h"
#include "ui_avform.h"
#include "src/video/videomode.h"

#include <memory>

class IAudioControl;
class IAudioSettings;
class IAudioSink;
class IAudioSource;
class CameraSource;
class CoreAV;
class IVideoSettings;
class VideoSurface;
class AVForm : public GenericForm, private Ui::AVForm
{
    Q_OBJECT
public:
    AVForm(IAudioControl& audio, CoreAV* coreAV, CameraSource& camera,
           IAudioSettings* audioSettings, IVideoSettings* videoSettings);
    ~AVForm() override;
    QString getFormName() final
    {
        return tr("Audio/Video");
    }

private:
    void getAudioInDevices();
    void getAudioOutDevices();
    void getVideoDevices();

    static int getModeSize(VideoMode mode);
    void selectBestModes(QVector<VideoMode>& allVideoModes);
    void fillCameraModesComboBox();
    void fillScreenModesComboBox();
    void fillAudioQualityComboBox();
    int searchPreferredIndex();

    void createVideoSurface();
    void killVideoSurface();

    void retranslateUi();

private slots:
    // audio
    void on_inDevCombobox_currentIndexChanged(int deviceIndex);
    void on_outDevCombobox_currentIndexChanged(int deviceIndex);
    void on_playbackSlider_valueChanged(int sliderSteps);
    void on_cbEnableTestSound_stateChanged();
    void on_microphoneSlider_valueChanged(int sliderSteps);
    void on_audioThresholdSlider_valueChanged(int sliderSteps);
    void on_audioQualityComboBox_currentIndexChanged(int index);

    // camera
    void on_videoDevCombobox_currentIndexChanged(int index);
    void on_videoModescomboBox_currentIndexChanged(int index);

    void rescanDevices();
    void setVolume(float value);

    void on_cbEnableBackend2_stateChanged();

protected:
    void updateVideoModes(int curIndex);

private:
    void hideEvent(QHideEvent* event) final;
    void showEvent(QShowEvent* event) final;
    void open(const QString& devName, const VideoMode& mode);
    int getStepsFromValue(qreal val, qreal valMin, qreal valMax);
    qreal getValueFromSteps(int steps, qreal valMin, qreal valMax);
    void trackNewScreenGeometry(QScreen* qScreen);

private:
    IAudioControl& audio;
    CoreAV* coreAV;
    IAudioSettings* audioSettings;
    IVideoSettings* videoSettings;

    bool subscribedToAudioIn;
    std::unique_ptr<IAudioSink> audioSink;
    std::unique_ptr<IAudioSource> audioSrc;
    VideoSurface* camVideoSurface;
    CameraSource& camera;
    QVector<QPair<QString, QString>> videoDeviceList;
    QVector<VideoMode> videoModes;
    uint alSource;
    const uint totalSliderSteps = 100; // arbitrary number of steps to give slider a good "feel"
};

#endif
