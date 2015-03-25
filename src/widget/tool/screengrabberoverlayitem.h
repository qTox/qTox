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

#ifndef SCREENGRABBEROVERLAYITEM_HPP
#define SCREENGRABBEROVERLAYITEM_HPP

#include <QGraphicsRectItem>

class ScreenshotGrabber;

class ScreenGrabberOverlayItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    ScreenGrabberOverlayItem(ScreenshotGrabber* grabber);
    ~ScreenGrabberOverlayItem();
    
    void setChosenRect(QRect rect);
    
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    
private:
    ScreenshotGrabber* screnshootGrabber;
    
    QRect chosenRect;
    
    
};

#endif // SCREENGRABBEROVERLAYITEM_HPP
