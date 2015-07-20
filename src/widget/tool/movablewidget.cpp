/*
    Copyright © 2015 by The qTox Project

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "movablewidget.h"
#include <QMouseEvent>

MovableWidget::MovableWidget(QWidget *parent)
    : QWidget(parent)
{

}

void MovableWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        moving = true;
        lastPoint = event->globalPos();
    }
}

void MovableWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (moving)
    {
        QPoint moveTo = pos() - (lastPoint - event->globalPos());

        if (moveTo.x() < 0)
            moveTo.setX(0);

        if (moveTo.y() < 0)
            moveTo.setY(0);

        if (moveTo.x() + width() > parentWidget()->width())
            moveTo.setX(parentWidget()->width() - width());

        if (moveTo.y() + height() > parentWidget()->height())
            moveTo.setY(parentWidget()->height() - height());

        move(moveTo);
        lastPoint = event->globalPos();
    }
}

void MovableWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton))
        moving = false;
}
