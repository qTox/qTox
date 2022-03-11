/*
    Copyright © 2015-2019 by The qTox Project Contributors

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
#include <QGraphicsOpacityEffect>
#include <QMouseEvent>
#include <cmath>

MovableWidget::MovableWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(64);
    setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
    actualSize = minimumSize();
    boundaryRect = QRect(0, 0, 0, 0);
    setRatio(1.0f);
    resize(minimumSize());
    actualPos = QPoint(0, 0);
}

void MovableWidget::resetBoundary(QRect newBoundary)
{
    boundaryRect = newBoundary;
    resize(QSize(round(actualSize.width()), round(actualSize.height())));

    QPoint moveTo = QPoint(round(actualPos.x()), round(actualPos.y()));
    checkBoundary(moveTo);
    move(moveTo);
    actualPos = moveTo;
}

void MovableWidget::setBoundary(QRect newBoundary)
{
    if (boundaryRect.isNull()) {
        boundaryRect = newBoundary;
        return;
    }

    qreal changeX = newBoundary.width() / static_cast<qreal>(boundaryRect.width());
    qreal changeY = newBoundary.height() / static_cast<qreal>(boundaryRect.height());

    qreal percentageX = (x() - boundaryRect.x()) / static_cast<qreal>(boundaryRect.width() - width());
    qreal percentageY =
        (y() - boundaryRect.y()) / static_cast<qreal>(boundaryRect.height() - height());

    actualSize.setWidth(actualSize.width() * changeX);
    actualSize.setHeight(actualSize.height() * changeY);

    if (actualSize.width() == 0)
        actualSize.setWidth(1);

    if (actualSize.height() == 0)
        actualSize.setHeight(1);

    resize(QSize(round(actualSize.width()), round(actualSize.height())));

    actualPos = QPointF(percentageX * (newBoundary.width() - width()),
                        percentageY * (newBoundary.height() - height()));
    actualPos += QPointF(newBoundary.topLeft());

    QPoint moveTo = QPoint(round(actualPos.x()), round(actualPos.y()));
    move(moveTo);

    boundaryRect = newBoundary;
}

float MovableWidget::getRatio() const
{
    return ratio;
}

void MovableWidget::setRatio(float r)
{
    ratio = r;
    resize(width(), width() / ratio);
    QPoint position = QPoint(actualPos.x(), actualPos.y());
    checkBoundary(position);
    move(position);

    actualPos = pos();
    actualSize = size();
}

void MovableWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton) {
        if (!(mode & Resize))
            mode |= Moving;

        lastPoint = event->globalPos();
    }
}

void MovableWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (mode & Moving) {
        QPoint moveTo = pos() - (lastPoint - event->globalPos());
        checkBoundary(moveTo);

        move(moveTo);
        lastPoint = event->globalPos();

        actualPos = pos();
    } else {
        if (!(event->buttons() & Qt::LeftButton)) {
            if (event->x() < 6)
                mode |= ResizeLeft;
            else
                mode &= ~ResizeLeft;

            if (event->y() < 6)
                mode |= ResizeUp;
            else
                mode &= ~ResizeUp;

            if (event->x() > width() - 6)
                mode |= ResizeRight;
            else
                mode &= ~ResizeRight;

            if (event->y() > height() - 6)
                mode |= ResizeDown;
            else
                mode &= ~ResizeDown;
        }

        if (mode & Resize) {
            const Modes ResizeUpRight = ResizeUp | ResizeRight;
            const Modes ResizeUpLeft = ResizeUp | ResizeLeft;
            const Modes ResizeDownRight = ResizeDown | ResizeRight;
            const Modes ResizeDownLeft = ResizeDown | ResizeLeft;

            if ((mode & ResizeUpRight) == ResizeUpRight || (mode & ResizeDownLeft) == ResizeDownLeft)
                setCursor(Qt::SizeBDiagCursor);
            else if ((mode & ResizeUpLeft) == ResizeUpLeft || (mode & ResizeDownRight) == ResizeDownRight)
                setCursor(Qt::SizeFDiagCursor);
            else if (mode & (ResizeLeft | ResizeRight))
                setCursor(Qt::SizeHorCursor);
            else
                setCursor(Qt::SizeVerCursor);

            if (event->buttons() & Qt::LeftButton) {
                QPoint lastPosition = pos();
                QPoint displacement = lastPoint - event->globalPos();
                QSize lastSize = size();


                if (mode & ResizeUp) {
                    lastSize.setHeight(height() + displacement.y());

                    if (lastSize.height() > maximumHeight())
                        lastPosition.setY(y() - displacement.y()
                                          + (lastSize.height() - maximumHeight()));
                    else
                        lastPosition.setY(y() - displacement.y());
                }

                if (mode & ResizeLeft) {
                    lastSize.setWidth(width() + displacement.x());
                    if (lastSize.width() > maximumWidth())
                        lastPosition.setX(x() - displacement.x() + (lastSize.width() - maximumWidth()));
                    else
                        lastPosition.setX(x() - displacement.x());
                }

                if (mode & ResizeRight)
                    lastSize.setWidth(width() - displacement.x());

                if (mode & ResizeDown)
                    lastSize.setHeight(height() - displacement.y());

                if (lastSize.height() > maximumHeight())
                    lastSize.setHeight(maximumHeight());

                if (lastSize.width() > maximumWidth())
                    lastSize.setWidth(maximumWidth());

                if (mode & (ResizeLeft | ResizeRight)) {
                    if (mode & (ResizeUp | ResizeDown)) {
                        int height = lastSize.width() / getRatio();

                        if (!(mode & ResizeDown))
                            lastPosition.setY(lastPosition.y() - (height - lastSize.height()));

                        resize(lastSize.width(), height);

                        if (lastSize.width() < minimumWidth())
                            lastPosition.setX(pos().x());

                        if (height < minimumHeight())
                            lastPosition.setY(pos().y());
                    } else {
                        resize(lastSize.width(), lastSize.width() / getRatio());
                    }
                } else {
                    resize(lastSize.height() * getRatio(), lastSize.height());
                }

                updateGeometry();

                checkBoundary(lastPosition);

                move(lastPosition);

                lastPoint = event->globalPos();
                actualSize = size();
                actualPos = pos();
            }
        } else {
            unsetCursor();
        }
    }
}

void MovableWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton))
        mode = 0;
}

void MovableWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;

    if (!graphicsEffect()) {
        QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(this);
        opacityEffect->setOpacity(0.5);
        setGraphicsEffect(opacityEffect);
    } else {
        setGraphicsEffect(nullptr);
    }
}

void MovableWidget::checkBoundary(QPoint& point) const
{
    int x1, y1, x2, y2;
    boundaryRect.getCoords(&x1, &y1, &x2, &y2);

    if (point.x() < boundaryRect.left())
        point.setX(boundaryRect.left());

    if (point.y() < boundaryRect.top())
        point.setY(boundaryRect.top());

    if (point.x() + width() > boundaryRect.right() + 1)
        point.setX(boundaryRect.right() - width() + 1);

    if (point.y() + height() > boundaryRect.bottom() + 1)
        point.setY(boundaryRect.bottom() - height() + 1);
}
