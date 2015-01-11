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

#include "chatlinecontentproxy.h"
#include <QLayout>
#include <QWidget>
#include <QDebug>

ChatLineContentProxy::ChatLineContentProxy(QWidget* widget, int minWidth, float widthInPercent)
    : widthMin(minWidth)
    , widthPercent(widthInPercent)
{
    proxy = new QGraphicsProxyWidget(this);
    proxy->setWidget(widget);
}

QRectF ChatLineContentProxy::boundingRect() const
{
    return proxy->boundingRect();
}

QRectF ChatLineContentProxy::boundingSceneRect() const
{
    return proxy->boundingRect().translated(scenePos());
}

void ChatLineContentProxy::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    proxy->paint(painter, option, widget);
}

qreal ChatLineContentProxy::getAscent() const
{
    return proxy->widget()->layout()->contentsMargins().top();
}

QWidget *ChatLineContentProxy::getWidget() const
{
    return proxy->widget();
}

void ChatLineContentProxy::setWidth(qreal width)
{
    proxy->widget()->setFixedWidth(qMax(static_cast<int>(width*widthPercent), widthMin));
}
