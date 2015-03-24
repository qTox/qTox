/* Copyright (c) 2014-2015, The Nuria Project
 * The NuriaProject Framework is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 * 
 * The NuriaProject Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with The NuriaProject Framework.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "screengrabberchooserrectitem.hpp"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QPainter>
#include <QCursor>

ScreenGrabberChooserRectItem::ScreenGrabberChooserRectItem()
{
    setCursor(QCursor(Qt::OpenHandCursor));
}

ScreenGrabberChooserRectItem::~ScreenGrabberChooserRectItem()
{
    
}

int ScreenGrabberChooserRectItem::width() const
{
    return this->rectWidth;
}

int ScreenGrabberChooserRectItem::height() const
{
    return this->rectHeight;
}

QRectF ScreenGrabberChooserRectItem::boundingRect() const
{
    return QRectF(0, 0, this->rectWidth, this->rectHeight);
}

void ScreenGrabberChooserRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    painter->save();
    
    QPen pen(Qt::blue);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(0, 0, this->rectWidth, this->rectHeight);
    
    painter->restore();
}

void ScreenGrabberChooserRectItem::beginResize()
{
    this->rectWidth = this->rectHeight = 0;
    this->state = Resizing;
    setCursor(QCursor(Qt::CrossCursor));
    grabMouse();
}

QRect ScreenGrabberChooserRectItem::chosenRect() const
{
    QRect rect (x(), y(), this->rectWidth, this->rectHeight);
    return rect.normalized();
}

void ScreenGrabberChooserRectItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        this->state = Moving;
        setCursor(QCursor(Qt::ClosedHandCursor));
    }
    
}

void ScreenGrabberChooserRectItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (this->state == Moving) {
        QPointF delta = event->scenePos() - event->lastScenePos();
        moveBy (delta.x(), delta.y());
    } else if (this->state == Resizing) {
        prepareGeometryChange();
        QPointF size = event->scenePos() - scenePos();
        this->rectWidth = size.x();
        this->rectHeight = size.y();
    }
    
    emit regionChosen(chosenRect());
    scene()->update();
}

void ScreenGrabberChooserRectItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        setCursor(QCursor(Qt::OpenHandCursor));
        emit regionChosen(chosenRect());
        this->state = None;
        ungrabMouse();
        
    }
    
}

void ScreenGrabberChooserRectItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    emit doubleClicked();
}
