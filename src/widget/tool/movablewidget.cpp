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

void MovableWidget::setBoundary(const QRect& boundary)
{
    boundaryRect = boundary;

    QPoint moveTo = pos();
    checkBoundary(moveTo);
    move(moveTo);
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
        checkBoundary(moveTo);

        move(moveTo);
        lastPoint = event->globalPos();
    }
}

void MovableWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton))
        moving = false;
}
#include <QGraphicsOpacityEffect>
void MovableWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;

    if (!graphicsEffect())
    {
        QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(this);
        opacityEffect->setOpacity(0.5);
        setGraphicsEffect(opacityEffect);
    }
    else
    {
        setGraphicsEffect(nullptr);
    }
}

void MovableWidget::checkBoundary(QPoint& point) const
{
    int x1, y1, x2, y2;
    boundaryRect.getCoords(&x1, &y1, &x2, &y2);

    // Video boundary.
    if (point.x() + width() < x1)
        point.setX(x1 - width());

    if (point.y() + height() <y1)
        point.setY(y1 - height());

    if (point.x() > x2)
        point.setX(x2);

    if (point.y() > y2)
        point.setY(y2);

    // Parent boundary.
    if (point.x() < 0)
        point.setX(0);

    if (point.y() < 0)
        point.setY(0);

    if (point.x() + width() > parentWidget()->width())
        point.setX(parentWidget()->width() - width());

    if (point.y() + height() > parentWidget()->height())
        point.setY(parentWidget()->height() - height());
}
