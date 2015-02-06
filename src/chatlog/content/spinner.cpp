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

#include "spinner.h"
#include "../pixmapcache.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QTime>
#include <QDebug>

Spinner::Spinner(const QString &img, QSize Size, qreal speed)
    : size(Size)
    , rotSpeed(speed)
{
    pmap = PixmapCache::getInstance().get(img, size);

    timer.setInterval(1000/30); // 30Hz
    timer.setSingleShot(false);

    QObject::connect(&timer, &QTimer::timeout, this, &Spinner::timeout);
}

QRectF Spinner::boundingRect() const
{
    return QRectF(QPointF(-size.width() / 2.0, -size.height() / 2.0), size);
}

void Spinner::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->setClipRect(boundingRect());

    QTransform rotMat;
    rotMat.translate(size.width() / 2.0, size.height() / 2.0);
    rotMat.rotate(QTime::currentTime().msecsSinceStartOfDay() / 1000.0 * rotSpeed);
    rotMat.translate(-size.width() / 2.0, -size.height() / 2.0);

    painter->translate(-size.width() / 2.0, -size.height() / 2.0);
    painter->setTransform(rotMat, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawPixmap(0, 0, pmap);

    Q_UNUSED(option)
    Q_UNUSED(widget)
}

void Spinner::setWidth(qreal width)
{
    Q_UNUSED(width)
}

void Spinner::visibilityChanged(bool visible)
{
    if(visible)
        timer.start();
    else
        timer.stop();
}

qreal Spinner::getAscent() const
{
    return 0.0;
}

void Spinner::timeout()
{
    if(scene())
        scene()->invalidate(sceneBoundingRect());
}
