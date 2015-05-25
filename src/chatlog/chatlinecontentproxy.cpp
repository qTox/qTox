/*
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
#include <QPainter>
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
    QRectF result = proxy->boundingRect();
    result.setHeight(result.height() + 5);
    return result;
}

void ChatLineContentProxy::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setClipRect(boundingRect());
    proxy->paint(painter, option, widget);
}

qreal ChatLineContentProxy::getAscent() const
{
    return 7.0;
}

QWidget *ChatLineContentProxy::getWidget() const
{
    return proxy->widget();
}

void ChatLineContentProxy::setWidth(qreal width)
{
    proxy->widget()->setFixedWidth(qMax(static_cast<int>(width*widthPercent), widthMin));
}
