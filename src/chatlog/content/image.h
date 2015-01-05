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

#ifndef IMAGE_H
#define IMAGE_H

#include "../chatlinecontent.h"

class Image : public ChatLineContent
{
public:
    Image(QSizeF size, const QString &filename);

    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    virtual void setWidth(qreal width) override;
    virtual QRectF boundingSceneRect() const override;
    virtual qreal getAscent() const override;

private:
    QSizeF size;
    QPixmap pmap;

};

#endif // IMAGE_H
