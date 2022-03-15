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

#include "screengrabberoverlayitem.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>

#include "screenshotgrabber.h"

ScreenGrabberOverlayItem::ScreenGrabberOverlayItem(ScreenshotGrabber* grabber)
    : screnshootGrabber(grabber)
{

    QBrush overlayBrush(QColor(0x00, 0x00, 0x00, 0x70)); // Translucent black

    setCursor(QCursor(Qt::CrossCursor));
    setBrush(overlayBrush);
    setPen(QPen(Qt::NoPen));
}

ScreenGrabberOverlayItem::~ScreenGrabberOverlayItem()
{
}

void ScreenGrabberOverlayItem::setChosenRect(QRect rect)
{
    QRect oldRect = chosenRect;
    chosenRect = rect;
    update(oldRect.united(rect));
}

void ScreenGrabberOverlayItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        screnshootGrabber->beginRectChooser(event);
}

void ScreenGrabberOverlayItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    std::ignore = option;
    std::ignore = widget;
    painter->setBrush(brush());
    painter->setPen(pen());

    QRectF self = rect();
    qreal leftX = chosenRect.x();
    qreal rightX = chosenRect.x() + chosenRect.width();
    qreal topY = chosenRect.y();
    qreal bottomY = chosenRect.y() + chosenRect.height();

    painter->drawRect(0, 0, leftX, self.height());                      // Left of chosen
    painter->drawRect(rightX, 0, self.width() - rightX, self.height()); // Right of chosen
    painter->drawRect(leftX, 0, chosenRect.width(), topY);              // Top of chosen
    painter->drawRect(leftX, bottomY, chosenRect.width(),
                      self.height() - bottomY); // Bottom of chosen
}
