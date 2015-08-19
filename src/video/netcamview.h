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

#ifndef NETCAMVIEW_H
#define NETCAMVIEW_H

#include "genericnetcamview.h"

class QHBoxLayout;
struct vpx_image;
class VideoSource;
class QFrame;
class MovableWidget;

class NetCamView : public GenericNetCamView
{
    Q_OBJECT

public:
    NetCamView(int friendId, QWidget *parent=0);

    virtual void show(VideoSource* source, const QString& title);
    virtual void hide();

    void setSource(VideoSource* s);
    void setTitle(const QString& title);

protected:
    void showEvent(QShowEvent* event) final override;

private slots:
    void updateRatio();

private:
    void updateFrameSize(QSize size);

    VideoSurface* selfVideoSurface;
    MovableWidget* selfFrame;
    int friendId;
    bool e = false;
};

#endif // NETCAMVIEW_H
