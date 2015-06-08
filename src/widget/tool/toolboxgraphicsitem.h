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

#ifndef TOOLBOXGRAPHICSITEM_HPP
#define TOOLBOXGRAPHICSITEM_HPP

#include <QGraphicsItemGroup>
#include <QPropertyAnimation>
#include <QObject>

class ToolBoxGraphicsItem final : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    ToolBoxGraphicsItem();
    ~ToolBoxGraphicsItem();

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) final override;

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) final override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) final override;

private:

    void startAnimation(QAbstractAnimation::Direction direction);

    QPropertyAnimation* opacityAnimation;
    qreal idleOpacity = 0.7f;
    qreal activeOpacity = 1.0f;
    int fadeTimeMs = 300;
};

#endif // TOOLBOXGRAPHICSITEM_HPP
