/*
    Copyright (C) 2015 by Project Tox <https://tox.im>

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

#ifndef SCREENSHODIALOG_H
#define SCREENSHODIALOG_H

#include <QObject>
#include <QPixmap>
#include <QPoint>

class QEventLoop;
class QRubberBand;

class ScreenshotGrabber : public QObject
{
    Q_OBJECT
public:
    enum
    {
        Rejected = 0,
        Accepted = 1
    };
    ScreenshotGrabber(QWidget* parent);
    bool eventFilter(QObject*, QEvent* event);
    int exec();
signals:
    void screenshotTaken(QPixmap pixmap);
private:
    void takeScreenshot(const QRect &selection);
    enum Status : uint8_t
    {
        Dead,
        Check, // Checking whether mouse moved far enough.
        Alive,
        Finished // Already submitted screenshot.
    };
    QWidget* parentWidget;
    QEventLoop* eventLoop;
    QRubberBand* rubberBand;
    QPoint point;
    Status status;
};
#endif // SCREENSHODIALOG_H

