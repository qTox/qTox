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

#include "screenshotgrabber.h"

#include <QEvent>
#include <QMouseEvent>
#include <QEventLoop>
#include <QRubberBand>
#include <QApplication>
#include <QScreen>
#include <QTimer>

#include <cassert>

void normalizeRect(const QPoint &position, QPoint &point, QRect &rect)
{
    int w, h, x, y;
    w = position.x() - point.x();
    h = position.y() - point.y();

    if (w >= 0)
    {
        x = point.x();
    }
    else
    {
        x = point.x() + w;
        w = -w;
    }
    if (h >= 0)
    {
        y = point.y();
    }
    else
    {
        y = point.y() + h;
        h = -h;
    }
    rect.setRect(x, y, w, h);
}

ScreenshotGrabber::ScreenshotGrabber(QWidget* parent)
    : QObject(parent),
    parentWidget(parent),
    rubberBand(nullptr),
    status(Dead)
{
    assert(parent != nullptr);
}

int ScreenshotGrabber::exec()
{
    if (parentWidget == nullptr)
    {
        return Rejected;
    }
    parentWidget->grabMouse();

    QApplication::setOverrideCursor(Qt::CrossCursor);

    parentWidget->installEventFilter(this);
    QEventLoop loop(this);
    eventLoop = &loop;
    parentWidget->setMouseTracking(true);
    int result = eventLoop->exec();
    parentWidget->removeEventFilter(this);

    QApplication::setOverrideCursor(Qt::ArrowCursor);
    return result;
}

bool ScreenshotGrabber::eventFilter(QObject* , QEvent* event)
{
    switch (event->type())
    {
    case QEvent::MouseButtonPress:
    {
        QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton && status != Finished)
        {
            point = parentWidget->mapToGlobal(QPoint(mouseEvent->x(), mouseEvent->y()));
            status = Check;
        }
        break;
    }
    case QEvent::MouseButtonRelease:
    {
        QRect rekt;
        if (rubberBand != nullptr)
        {
            rekt = rubberBand->geometry();
            delete rubberBand;
            rubberBand = nullptr;
            QGuiApplication::processEvents();
        }
        QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
        if (mouseEvent->button() != Qt::LeftButton)
        {
            parentWidget->releaseMouse();
            eventLoop->exit(Rejected);
        }
        if (status != Alive || mouseEvent->pos() == point)
        {
            status = Dead;
            break;
        }

        parentWidget->releaseMouse();

        QTimer::singleShot(100, this, [=]()
        {
            takeScreenshot(rekt);
        });

        break;
    }
    case QEvent::MouseMove:
    {
        QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
        if (rubberBand != nullptr)
        {
            assert(status == Alive);

            QRect rect;
            normalizeRect(parentWidget->mapToGlobal(mouseEvent->pos()), point, rect);

            rubberBand->setGeometry(rect);
            QGuiApplication::processEvents();
        }
        else if (status == Check)
        {
            if (mouseEvent->pos() != point)
            {
                rubberBand = new QRubberBand(QRubberBand::Rectangle);
                rubberBand->setGeometry(QRect(parentWidget->mapToGlobal(mouseEvent->pos()), QPoint(0, 0)));
                rubberBand->show();
                QGuiApplication::processEvents();
                status = Alive;
            }
        }
        break;
    }
    default:
        break;
    }
    return false;
}

void ScreenshotGrabber::takeScreenshot(const QRect &selection)
{
    QScreen* screen = QApplication::primaryScreen();
    emit screenshotTaken(screen->grabWindow(0, selection.x(), selection.y(), selection.width(), selection.height()));

    eventLoop->exit(Accepted);
}
