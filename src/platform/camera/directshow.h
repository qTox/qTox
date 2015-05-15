#ifndef DIRECTSHOW_H
#define DIRECTSHOW_H

#include <QString>
#include <QVector>
#include <QPair>

#ifndef Q_OS_WIN
#error "This file is only meant to be compiled for Windows targets"
#endif

class DirectShow
{
public:
    static QVector<QPair<QString,QString>> getDeviceList();

private:
    DirectShow()=delete;
};

#endif // DIRECTSHOW_H
