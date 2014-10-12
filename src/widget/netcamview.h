/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

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

class QCloseEvent;
class QShowEvent;
class QPainter;
class QLabel;
class QHBoxLayout;
class QImage;
class vpx_image;

class NetCamView : public QWidget
{
    Q_OBJECT

public:
    NetCamView(QWidget *parent=0);

public slots:
    void updateDisplay(vpx_image* frame);

private:
    static QImage convert(vpx_image& frame);

protected:
    void resizeEvent(QResizeEvent *e);

private:
    QLabel *displayLabel;
    QImage lastFrame;
    QHBoxLayout* mainLayout;
    QImage img;
};

#endif // NETCAMVIEW_H
