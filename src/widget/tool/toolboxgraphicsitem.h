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

#ifndef TOOLBOXGRAPHICSITEM_HPP
#define TOOLBOXGRAPHICSITEM_HPP

#include <QGraphicsItemGroup>
#include <QPropertyAnimation>
#include <QObject>

class ToolBoxGraphicsItem : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    ToolBoxGraphicsItem();
    ~ToolBoxGraphicsItem();
    
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    
protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
    
private:
    
    void startAnimation(QAbstractAnimation::Direction direction);
    
    QPropertyAnimation* opacityAnimation;
    qreal idleOpacity = 0.7f;
    qreal activeOpacity = 1.0f;
    int fadeTimeMs = 300;
    
};

#endif // TOOLBOXGRAPHICSITEM_HPP
