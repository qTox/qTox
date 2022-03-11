/*
    Copyright © 2015-2019 by The qTox Project Contributors

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

#include "toolboxgraphicsitem.h"

#include <QPainter>

ToolBoxGraphicsItem::ToolBoxGraphicsItem()
{
    opacityAnimation = new QPropertyAnimation(this, QByteArrayLiteral("opacity"), this);

    opacityAnimation->setKeyValueAt(0, idleOpacity);
    opacityAnimation->setKeyValueAt(1, activeOpacity);
    opacityAnimation->setDuration(fadeTimeMs);

    setOpacity(activeOpacity);
}

ToolBoxGraphicsItem::~ToolBoxGraphicsItem()
{
}

void ToolBoxGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    startAnimation(QAbstractAnimation::Backward);
    QGraphicsItemGroup::hoverEnterEvent(event);
}

void ToolBoxGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    startAnimation(QAbstractAnimation::Forward);
    QGraphicsItemGroup::hoverLeaveEvent(event);
}

void ToolBoxGraphicsItem::startAnimation(QAbstractAnimation::Direction direction)
{
    opacityAnimation->setDirection(direction);
    opacityAnimation->start();
}

void ToolBoxGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                                QWidget* widget)
{
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(QColor(0xFF, 0xE2, 0x82)));
    painter->drawRect(childrenBoundingRect());
    painter->restore();

    QGraphicsItemGroup::paint(painter, option, widget);
}
