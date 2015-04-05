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

#include "toolboxgraphicsitem.h"

#include <QPainter>

ToolBoxGraphicsItem::ToolBoxGraphicsItem()
{
    this->opacityAnimation = new QPropertyAnimation(this, QByteArrayLiteral("opacity"), this);
    
    this->opacityAnimation->setKeyValueAt(0, this->idleOpacity);
    this->opacityAnimation->setKeyValueAt(1, this->activeOpacity);
    this->opacityAnimation->setDuration(this->fadeTimeMs);
    
    setOpacity(this->idleOpacity);
}

ToolBoxGraphicsItem::~ToolBoxGraphicsItem()
{
    
}

void ToolBoxGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    startAnimation(QAbstractAnimation::Forward);
    QGraphicsItemGroup::hoverEnterEvent(event);
}

void ToolBoxGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    startAnimation(QAbstractAnimation::Backward);
    QGraphicsItemGroup::hoverLeaveEvent(event);
}

void ToolBoxGraphicsItem::startAnimation(QAbstractAnimation::Direction direction)
{
    this->opacityAnimation->setDirection(direction);
    this->opacityAnimation->start();
}

void ToolBoxGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(QColor(0xFF, 0xE2, 0x82)));
    painter->drawRect(childrenBoundingRect());
    painter->restore();
    
    QGraphicsItemGroup::paint(painter, option, widget);
}
