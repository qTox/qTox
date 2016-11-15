/*
    Copyright © 2015 by The qTox Project

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

#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "video/videosurface.h"

class GenericNetCamView : public QWidget
{
    Q_OBJECT
public:
    explicit GenericNetCamView(QWidget* parent);
    QSize getSurfaceMinSize();

signals:
    void showMessageClicked();

public slots:
    void setShowMessages(bool show, bool notify = false);

protected:
    QVBoxLayout* verLayout;
    VideoSurface* videoSurface;

private:
    QPushButton* button;
};

#endif // GENERICNETCAMVIEW_H
