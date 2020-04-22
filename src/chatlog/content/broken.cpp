/*
    Copyright © 2019 by The qTox Project Contributors

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

#include "broken.h"
#include "src/chatlog/pixmapcache.h"
#include <QPainter>

class QStyleOptionGraphicsItem;

Broken::Broken(const QString& img, QSize size)
    : pmap{PixmapCache::getInstance().get(img, size)}
    , size{size}
{
}

QRectF Broken::boundingRect() const
{
    return QRectF(QPointF(-size.width() / 2.0, -size.height() / 2.0), size);
}

void Broken::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                       QWidget* widget)
{
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawPixmap(0, 0, pmap);

    Q_UNUSED(option)
    Q_UNUSED(widget)

}

void Broken::setWidth(qreal width)
{
    Q_UNUSED(width)
}

void Broken::visibilityChanged(bool visible)
{
    Q_UNUSED(visible)
}

qreal Broken::getAscent() const
{
    return 0.0;
}
