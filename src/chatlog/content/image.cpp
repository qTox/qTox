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

#include "image.h"

#include <QPainter>

Image::Image(QSizeF Size, const QString& filename)
    : size(Size)
{
    pmap.load(filename);
}

QRectF Image::boundingRect() const
{
    return QRectF(QPointF(-size.width() / 2.0, -size.height() / 2.0), size);
}

QRectF Image::boundingSceneRect() const
{
    return QRectF(scenePos(), size);
}

qreal Image::firstLineVOffset() const
{
    return size.height() / 4.0;
}

void Image::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->translate(-size.width() / 2.0, -size.height() / 2.0);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawPixmap(0, 0, size.width(), size.height(), pmap);

    Q_UNUSED(option)
    Q_UNUSED(widget)
}

void Image::setWidth(qreal width)
{
    Q_UNUSED(width)
}
