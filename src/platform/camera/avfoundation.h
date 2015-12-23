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
#ifndef AVFOUNDATION_H
#define AVFOUNDATION_H

#include <QString>
#include <QVector>
#include <QPair>
#include "src/video/videomode.h"

#ifndef Q_OS_MACX
#error "This file is only meant to be compiled for Mac OS X targets"
#endif

namespace avfoundation
{
    const QString CAPTURE_SCREEN{"Capture screen"};

    QVector<VideoMode> getDeviceModes(QString devName);
    QVector<QPair<QString, QString>> getDeviceList();
}

#endif // AVFOUNDATION_H
