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

#ifndef NOTIFICATIONICON_H
#define NOTIFICATIONICON_H

#include "../chatlinecontent.h"

#include <QLinearGradient>
#include <QPixmap>

class QTimer;

class NotificationIcon : public QObject,  public ChatLineContent
{
    Q_OBJECT
public:
    NotificationIcon(QSize size);

    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    virtual void setWidth(qreal width) override;
    virtual qreal getAscent() const override;

private slots:
    void updateGradient();

private:
    QSize size;
    QPixmap pmap;
    QLinearGradient grad;
    QTimer* updateTimer = nullptr;

    qreal dotWidth = 0.2;
    qreal alpha = 0.0;

};

#endif // NOTIFICATIONICON_H
