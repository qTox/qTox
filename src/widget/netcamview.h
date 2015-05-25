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

#ifndef NETCAMVIEW_H
#define NETCAMVIEW_H

#include <QWidget>

class QHBoxLayout;
struct vpx_image;
class VideoSurface;
class VideoSource;

class NetCamView : public QWidget
{
    Q_OBJECT

public:
    NetCamView(QWidget *parent=0);

    virtual void show(VideoSource* source, const QString& title);
    virtual void hide();

    void setSource(VideoSource* s);
    void setTitle(const QString& title);

private:
    QHBoxLayout* mainLayout;
    VideoSurface* videoSurface;
};

#endif // NETCAMVIEW_H
