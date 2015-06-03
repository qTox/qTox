#ifndef V4L2_H
#define V4L2_H

#include <QString>
#include <QVector>
#include <QPair>
#include "src/video/videomode.h"

#ifndef Q_OS_LINUX
#error "This file is only meant to be compiled for Linux targets"
#endif

namespace v4l2
{
    QVector<VideoMode> getDeviceModes(QString devName);
}

#endif // V4L2_H

