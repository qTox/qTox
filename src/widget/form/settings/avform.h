/*
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

#ifndef AVFORM_H
#define AVFORM_H

#include "genericsettings.h"
#include "src/widget/videosurface.h"
#include "src/video/camera.h"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QPushButton>

namespace Ui {
class AVSettings;
}

class Camera;

class AVForm : public GenericForm
{
    Q_OBJECT
public:
    AVForm();
    ~AVForm();
    virtual void present();

private:
    void getAudioInDevices();
    void getAudioOutDevices();

    void createVideoSurface();
    void killVideoSurface();

private slots:
    void on_ContrastSlider_sliderMoved(int position);
    void on_SaturationSlider_sliderMoved(int position);
    void on_BrightnessSlider_sliderMoved(int position);
    void on_HueSlider_sliderMoved(int position);
    void on_videoModescomboBox_currentIndexChanged(int index);

    // audio
    void onInDevChanged(const QString& deviceDescriptor);
    void onOutDevChanged(const QString& deviceDescriptor);
    void onFilterAudioToggled(bool filterAudio);
    void on_playbackSlider_valueChanged(int value);
    void on_microphoneSlider_valueChanged(int value);

    // camera
    void onPropProbingFinished(Camera::Prop prop, double val);
    void onResProbingFinished(QList<QSize> res);

    virtual void hideEvent(QHideEvent*);
    virtual void showEvent(QShowEvent*);

    void on_HueSlider_valueChanged(int value);
    void on_BrightnessSlider_valueChanged(int value);
    void on_SaturationSlider_valueChanged(int value);
    void on_ContrastSlider_valueChanged(int value);
    
protected:
    bool eventFilter(QObject *o, QEvent *e);    

private:
    Ui::AVSettings *bodyUI;
    VideoSurface* CamVideoSurface;
};

#endif
