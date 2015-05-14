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

#include <QObject>
#include <QList>
#include "genericsettings.h"

namespace Ui {
class AVSettings;
}

class CameraSource;
class VideoSurface;

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
    void getVideoDevices();

    void createVideoSurface();
    void killVideoSurface();

private slots:
    void on_videoModescomboBox_currentIndexChanged(int index);

    // audio
    void onInDevChanged(const QString& deviceDescriptor);
    void onOutDevChanged(const QString& deviceDescriptor);
    void onFilterAudioToggled(bool filterAudio);
    void on_playbackSlider_valueChanged(int value);
    void on_microphoneSlider_valueChanged(int value);

    // camera
    void onVideoDevChanged(int index);
    void onResProbingFinished(QList<QSize> res);

    virtual void hideEvent(QHideEvent*);
    virtual void showEvent(QShowEvent*);
    
protected:
    bool eventFilter(QObject *o, QEvent *e);    

private:
    Ui::AVSettings *bodyUI;
    VideoSurface* camVideoSurface;
    CameraSource* camera;
    QVector<QPair<QString, QString>> videoDeviceList;
};

#endif
