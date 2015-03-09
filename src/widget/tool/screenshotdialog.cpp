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

#include "screenshotdialog.h"
#include <QRubberBand>
#include <QMouseEvent>
#include <cassert>

ScreenshotDialog::ScreenshotDialog(QRect &region)
    : QDialog(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint),
    region(region),
    rubberBand(nullptr),
    status(Dead)
{
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    showMaximized();
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
}

ScreenshotDialog::~ScreenshotDialog()
{

}

void ScreenshotDialog::mousePressEvent(QMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton)
    {
        point = QPoint(mouseEvent->x(), mouseEvent->y());
        status = Check;
    }
}

void ScreenshotDialog::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
    if (rubberBand != nullptr)
    {
        rubberBand->deleteLater();
        rubberBand = nullptr;
    }
    if (mouseEvent->button() != Qt::LeftButton)
    {
        reject();
    }
    if (status != Alive || mouseEvent->pos() == point)
    {
        status = Dead;
        return;
    }
    calculateRect(mouseEvent, region);
    region = QRect(mapToGlobal(region.topLeft()), region.size());
    accept();
}

void ScreenshotDialog::mouseMoveEvent(QMouseEvent *mouseEvent)
{
    if (rubberBand != nullptr)
    {
        assert(status == Alive);

        QRect rect;
        calculateRect(mouseEvent, rect);

        rubberBand->setGeometry(rect);
    }
    else if (status == Check)
    {
        if (mouseEvent->pos() != point)
        {
            rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
            rubberBand->setGeometry(mouseEvent->x(), mouseEvent->y(), 1, 1);
            rubberBand->show();
            status = Alive;
        }
    }
}

void ScreenshotDialog::calculateRect(QMouseEvent *mouseEvent, QRect &rect)
{
    // Must normalize, since Qt doesn't support negative sizes.
    int w, h, x, y;
    w = mouseEvent->x() - point.x();
    h = mouseEvent->y() - point.y();

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


