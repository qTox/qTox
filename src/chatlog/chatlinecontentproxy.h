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

#ifndef CHATLINECONTENTPROXY_H
#define CHATLINECONTENTPROXY_H

#include <QGraphicsProxyWidget>
#include "chatlinecontent.h"

class ChatLineContentProxy : public ChatLineContent
{
public:
    ChatLineContentProxy(QWidget* widget, int minWidth, float widthInPercent = 1.0f);

    virtual QRectF boundingRect() const;
    virtual void setWidth(qreal width);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual qreal getAscent() const;

    QWidget* getWidget() const;

private:
    QGraphicsProxyWidget* proxy;
    int widthMin;
    float widthPercent;
};

#endif // CHATLINECONTENTPROXY_H
