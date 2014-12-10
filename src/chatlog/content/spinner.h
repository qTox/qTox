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

#ifndef SPINNER_H
#define SPINNER_H

#include "../chatlinecontent.h"

#include <QTimer>
#include <QObject>

class Spinner : public QObject, public ChatLineContent
{
    Q_OBJECT
public:
    Spinner(QSizeF size);

    virtual QRectF boundingRect() const;
    virtual QRectF boundingSceneRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void setWidth(qreal width);
    virtual void visibilityChanged(bool visible);
    virtual qreal getAscent() const;

private slots:
    void timeout();

private:
    QSizeF size;
    QPixmap pmap;
    qreal rot;
    QTimer timer;

};

#endif // SPINNER_H
