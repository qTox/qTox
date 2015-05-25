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

#include "adjustingscrollarea.h"

#include <QEvent>
#include <QLayout>
#include <QScrollBar>
#include <QDebug>

AdjustingScrollArea::AdjustingScrollArea(QWidget *parent)
    : QScrollArea(parent)
{

}

void AdjustingScrollArea::resizeEvent(QResizeEvent *ev)
{
    int scrollBarWidth = verticalScrollBar()->isVisible() ? verticalScrollBar()->sizeHint().width() : 0;

    if (layoutDirection() == Qt::RightToLeft)
        setViewportMargins(-scrollBarWidth, 0, 0, 0);

    updateGeometry();
    QScrollArea::resizeEvent(ev);
}

QSize AdjustingScrollArea::sizeHint() const
{
    if (widget())
    {
        int scrollbarWidth = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;
        return widget()->sizeHint() + QSize(scrollbarWidth, 0);
    }

    return QScrollArea::sizeHint();
}
