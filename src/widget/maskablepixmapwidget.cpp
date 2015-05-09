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
    , maskName(maskName)
    , backgroundColor(Qt::white)
    , clickable(false)
{
    setSize(size);
}

MaskablePixmapWidget::~MaskablePixmapWidget()
{
    delete renderTarget;
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
    manualColor = false;

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
        unscaled = pmap;
        pixmap = pmap.scaled(width() - 2, height() - 2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        backgroundColor = background;
        manualColor = true;
        update();
    }
}

void MaskablePixmapWidget::setPixmap(const QPixmap &pmap)
{
    if (!pmap.isNull())
    {
        unscaled = pmap;
        pixmap = pmap.scaled(width() - 2, height() - 2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        autopickBackground();
        update();
    }
}

QPixmap MaskablePixmapWidget::getPixmap() const
{
    return *renderTarget;
}

void MaskablePixmapWidget::setSize(QSize size)
{
    setFixedSize(size);
    delete renderTarget;
    renderTarget = new QPixmap(size);

    QPixmap pmapMask = QPixmap(maskName);
    if (!pmapMask.isNull())
        mask = pmapMask.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    if (!unscaled.isNull())
    {
        pixmap = unscaled.scaled(width() - 2, height() - 2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        if (!manualColor)
            autopickBackground();
        update();
    }
}

void MaskablePixmapWidget::paintEvent(QPaintEvent *)
{
    renderTarget->fill(Qt::transparent);

    QPoint offset((width() - pixmap.size().width())/2,(height() - pixmap.size().height())/2); // centering the pixmap

    QPainter painter(renderTarget);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.fillRect(0,0,width(),height(),backgroundColor);
    painter.drawPixmap(offset,pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter.drawPixmap(0,0,mask);
    painter.end();

    painter.begin(this);
    painter.drawPixmap(0,0,*renderTarget);
}

void MaskablePixmapWidget::mousePressEvent(QMouseEvent*)
{
    if (clickable)
        emit clicked();
}
