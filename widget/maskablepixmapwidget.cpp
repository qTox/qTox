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

#include "maskablepixmapwidget.h"
#include <QPainter>

MaskablePixmapWidget::MaskablePixmapWidget(QWidget *parent, QSize size, QString maskName)
    : QWidget(parent)
    , backgroundColor(Qt::white)
    , clickable(false)
{
    setFixedSize(size);

    QPixmap pmapMask = QPixmap(maskName);

    if (!pmapMask.isNull())
        mask = QPixmap(maskName).scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void MaskablePixmapWidget::autopickBackground()
{
    QImage pic = pixmap.toImage();

    if (pic.isNull())
        return;

    int r = 0;
    int g = 0;
    int b = 0;
    int weight = 0;

    for (int x=0;x<pic.width();++x)
    {
        for (int y=0;y<pic.height();++y)
        {
            QRgb color = pic.pixel(x,y);
            r += qRed(color);
            g += qGreen(color);
            b += qBlue(color);

            weight += qAlpha(color);
        }
    }

    weight = qMax(1, weight / 255);
    r /= weight;
    g /= weight;
    b /= weight;

    QColor color = QColor::fromRgb(r,g,b);
    backgroundColor =  QColor::fromRgb(0xFFFFFF ^ color.rgb());

    update();
}

void MaskablePixmapWidget::setBackground(QColor color)
{
    backgroundColor = color;
    update();
}

void MaskablePixmapWidget::setClickable(bool clickable)
{
    this->clickable = clickable;

    if (clickable)
        setCursor(Qt::PointingHandCursor);
    else
        unsetCursor();
}

void MaskablePixmapWidget::setPixmap(const QPixmap &pmap, QColor background)
{
    if (!pmap.isNull())
    {
        pixmap = pmap.scaled(width(), height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        backgroundColor = background;

        update();
    }
}

void MaskablePixmapWidget::setPixmap(const QPixmap &pmap)
{
    if (!pmap.isNull())
    {
        pixmap = pmap.scaled(width(), height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        autopickBackground();

        update();
    }
}

QPixmap MaskablePixmapWidget::getPixmap() const
{
    return pixmap;
}

void MaskablePixmapWidget::paintEvent(QPaintEvent *)
{
    QPixmap tmp(width(), height());
    tmp.fill(Qt::transparent);

    QPoint offset((width() - pixmap.size().width())/2,(height() - pixmap.size().height())/2); // centering the pixmap

    QPainter painter(&tmp);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.fillRect(0,0,width(),height(),backgroundColor);
    painter.drawPixmap(offset,pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter.drawPixmap(0,0,mask);
    painter.end();

    painter.begin(this);
    painter.drawPixmap(0,0,tmp);
}

void MaskablePixmapWidget::mousePressEvent(QMouseEvent*)
{
    if(clickable)
        emit clicked();
}
