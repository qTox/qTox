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
#include <QPaintEvent>

MaskablePixmapWidget::MaskablePixmapWidget(QWidget *parent, QSize size, QString maskName)
    : QWidget(parent)
{
    setMinimumSize(size);
    setMaximumSize(size);

    mask = QPixmap(maskName).scaled(maximumSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void MaskablePixmapWidget::setPixmap(const QPixmap &pmap)
{
    pixmap = pmap.scaled(maximumSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QPixmap MaskablePixmapWidget::getPixmap() const
{
    return pixmap;
}

void MaskablePixmapWidget::paintEvent(QPaintEvent *ev)
{
    QPixmap tmp(ev->rect().size());
    tmp.fill(Qt::transparent);

    QPoint offset((ev->rect().size().width()-pixmap.size().width())/2,(ev->rect().size().height()-pixmap.size().height())/2);

    QPainter painter(&tmp);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawPixmap(offset,pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter.drawPixmap(0,0,mask);

    painter.end();
    painter.begin(this);
    painter.drawPixmap(0,0,tmp);
}

void MaskablePixmapWidget::mousePressEvent(QMouseEvent*)
{
    emit clicked();
}
