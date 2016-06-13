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

#ifndef AVFORM_H
#define AVFORM_H

#include <QObject>
#include <QString>
#include <QList>
#include "genericsettings.h"
#include "src/video/videomode.h"

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
    QString getFormName() final override {return tr("Audio/Video");}

private:
    void getAudioInDevices();
    void getAudioOutDevices();
    void getVideoDevices();

    void selectBestModes(QVector<VideoMode> &allVideoModes);
    int fillModesComboBox();

    void createVideoSurface();
    void killVideoSurface();

    void retranslateUi();

private slots:

    // audio
    void onInDevChanged(QString deviceDescriptor);
    void onOutDevChanged(QString deviceDescriptor);
    void onPlaybackValueChanged(int value);
    void onMicrophoneValueChanged(int value);

    // camera
    void onVideoDevChanged(int index);
    void onVideoModesIndexChanged(int index);

    void on_btnPlayTestSound_clicked(bool checked);

protected:
    void updateVideoModes(int curIndex);

private:
    bool eventFilter(QObject *o, QEvent *e) final override;
    void hideEvent(QHideEvent* event) final override;
    void showEvent(QShowEvent*event) final override;

private:
    Ui::AVSettings *bodyUI;
    bool subscribedToAudioIn;
    bool mPlayTestSound;
    VideoSurface *camVideoSurface;
    CameraSource &camera;
    QVector<QPair<QString, QString>> videoDeviceList;
    QVector<VideoMode> videoModes;
};

#endif
