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
#include <QIcon>

class Spinner : public QObject, public ChatLineContent
{
    Q_OBJECT
public:
    Spinner(const QString& img, QSizeF size, qreal speed);

    virtual QRectF boundingRect() const override;
    virtual QRectF boundingSceneRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    virtual void setWidth(qreal width) override;
    virtual void visibilityChanged(bool visible) override;
    virtual qreal getAscent() const override;

private slots:
    void timeout();

private:
    QSizeF size;
    QIcon icon;
    qreal rot = 0.0;
    qreal rotSpeed;
    QTimer timer;

};

#endif // SPINNER_H
