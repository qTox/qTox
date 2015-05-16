#ifndef DIRECTSHOW_H
#define DIRECTSHOW_H

#include <QString>
#include <QVector>
#include <QPair>
#include "src/video/videomode.h"

#ifndef Q_OS_WIN
#error "This file is only meant to be compiled for Windows targets"
#endif

namespace DirectShow
{
    QVector<QPair<QString,QString>> getDeviceList();
    QVector<VideoMode> getDeviceModes(QString devName);
}

#endif // DIRECTSHOW_H
