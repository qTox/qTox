/*
    Copyright © 2015-2018 by The qTox Project Contributors

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

#ifndef GENERICNETCAMVIEW_H
#define GENERICNETCAMVIEW_H

#include <QFrame>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "src/video/videosurface.h"
#include "src/widget/style.h"

class GenericNetCamView : public QWidget
{
    Q_OBJECT
public:
    explicit GenericNetCamView(QWidget* parent);
    QSize getSurfaceMinSize();

signals:
    void showMessageClicked();
    void videoCallEnd();
    void volMuteToggle();
    void micMuteToggle();
    void videoPreviewToggle();

public slots:
    void setShowMessages(bool show, bool notify = false);
    void updateMuteVolButton(bool isMuted);
    void updateMuteMicButton(bool isMuted);

protected:
    QVBoxLayout* verLayout;
    VideoSurface* videoSurface;
    QPushButton* enterFullScreenButton = nullptr;

private:
    QHBoxLayout* buttonLayout = nullptr;
    QPushButton* toggleMessagesButton = nullptr;
    QFrame* buttonPanel = nullptr;
    QPushButton* videoPreviewButton = nullptr;
    QPushButton* volumeButton = nullptr;
    QPushButton* microphoneButton = nullptr;
    QPushButton* endVideoButton = nullptr;
    QPushButton* exitFullScreenButton = nullptr;

private:
    QPushButton* createButton(const QString& name, const QString& state);
    void toggleFullScreen();
    void enterFullScreen();
    void exitFullScreen();
    void endVideoCall();
    void toggleVideoPreview();
    void toggleButtonState(QPushButton* btn);
    void updateButtonState(QPushButton* btn, bool active);
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
};

#endif // GENERICNETCAMVIEW_H
