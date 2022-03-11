/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#include "image.h"
#include "../pixmapcache.h"

#include <QPainter>

Image::Image(QSize Size, const QString& filename)
    : size(Size)
{
    pmap = PixmapCache::getInstance().get(filename, size);
}

QRectF Image::boundingRect() const
{
    return QRectF(QPointF(-size.width() / 2.0, -size.height() / 2.0), size);
}

qreal Image::getAscent() const
{
    return 0.0;
}

void Image::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->setClipRect(boundingRect());

    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->translate(-size.width() / 2.0, -size.height() / 2.0);
    painter->drawPixmap(0, 0, pmap);

    std::ignore = option;
    std::ignore = widget;
}

void Image::setWidth(float width)
{
    std::ignore = width;
}
