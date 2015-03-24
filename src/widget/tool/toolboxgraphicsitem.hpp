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
    
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    
protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    
private:
    
    void startAnimation(QAbstractAnimation::Direction direction);
    
    QPropertyAnimation *opacityAnimation;
    qreal idleOpacity = 0.7f;
    qreal activeOpacity = 1.0f;
    int fadeTimeMs = 300;
    
};

#endif // TOOLBOXGRAPHICSITEM_HPP
