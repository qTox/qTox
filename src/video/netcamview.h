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

#pragma once

#include "src/core/toxpk.h"
#include <QVector>
#include <QWidget>

class QHBoxLayout;
struct vpx_image;
class VideoSource;
class QFrame;
class MovableWidget;
class QVBoxLayout;
class VideoSurface;
class QPushButton;
class QKeyEvent;
class QCloseEvent;
class QShowEvent;

class NetCamView : public QWidget
{
    Q_OBJECT

public:
    NetCamView(ToxPk friendPk, QWidget* parent = nullptr);
    ~NetCamView();

    virtual void show(VideoSource* source, const QString& title);
    virtual void hide();

    void setSource(VideoSource* s);
    void setTitle(const QString& title);
    QSize getSurfaceMinSize();

protected:
    void showEvent(QShowEvent* event);
    QVBoxLayout* verLayout;
    VideoSurface* videoSurface;
    QPushButton* enterFullScreenButton = nullptr;

signals:
    void showMessageClicked();
    void videoCallEnd();
    void volMuteToggle();
    void micMuteToggle();

public slots:
    void setShowMessages(bool show, bool notify = false);
    void updateMuteVolButton(bool isMuted);
    void updateMuteMicButton(bool isMuted);

private slots:
    void updateRatio();

private:
    void updateFrameSize(QSize size);
    QPushButton* createButton(const QString& name, const QString& state);
    void toggleFullScreen();
    void enterFullScreen();
    void exitFullScreen();
    void endVideoCall();
    void toggleVideoPreview();
    void toggleButtonState(QPushButton* btn);
    void updateButtonState(QPushButton* btn, bool active);
    void keyPressEvent(QKeyEvent *event);
    void closeEvent(QCloseEvent *event);
    VideoSurface* selfVideoSurface;
    MovableWidget* selfFrame;
    ToxPk friendPk;
    bool e;
    QVector<QMetaObject::Connection> connections;
    QHBoxLayout* buttonLayout = nullptr;
    QPushButton* toggleMessagesButton = nullptr;
    QFrame* buttonPanel = nullptr;
    QPushButton* videoPreviewButton = nullptr;
    QPushButton* volumeButton = nullptr;
    QPushButton* microphoneButton = nullptr;
    QPushButton* endVideoButton = nullptr;
    QPushButton* exitFullScreenButton = nullptr;
};
