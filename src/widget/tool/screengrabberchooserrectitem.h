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

#pragma once

#include <QGraphicsItemGroup>

class ScreenGrabberChooserRectItem final : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT
public:
    explicit ScreenGrabberChooserRectItem(QGraphicsScene* scene);
    ~ScreenGrabberChooserRectItem();

    QRectF boundingRect() const final;
    void beginResize(QPointF mousePos);

    QRect chosenRect() const;

    void showHandles();
    void hideHandles();

signals:

    void doubleClicked();
    void regionChosen(QRect rect);

protected:
    bool sceneEventFilter(QGraphicsItem* watched, QEvent* event) final;

private:
    enum State
    {
        None,
        Resizing,
        HandleResizing,
        Moving,
    };

    State state = None;
    int rectWidth = 0;
    int rectHeight = 0;
    QPointF startPos;

    void forwardMainRectEvent(QEvent* event);
    void forwardHandleEvent(QGraphicsItem* watched, QEvent* event);

    void mousePress(QGraphicsSceneMouseEvent* event);
    void mouseMove(QGraphicsSceneMouseEvent* event);
    void mouseRelease(QGraphicsSceneMouseEvent* event);
    void mouseDoubleClick(QGraphicsSceneMouseEvent* event);

    void mousePressHandle(int x, int y, QGraphicsSceneMouseEvent* event);
    void mouseMoveHandle(int x, int y, QGraphicsSceneMouseEvent* event);
    void mouseReleaseHandle(int x, int y, QGraphicsSceneMouseEvent* event);

    QPoint getHandleMultiplier(QGraphicsItem* handle);

    void updateHandlePositions();
    QGraphicsRectItem* createHandleItem(QGraphicsScene* scene);

    QGraphicsRectItem* mainRect;
    QGraphicsRectItem* topLeft;
    QGraphicsRectItem* topCenter;
    QGraphicsRectItem* topRight;
    QGraphicsRectItem* rightCenter;
    QGraphicsRectItem* bottomRight;
    QGraphicsRectItem* bottomCenter;
    QGraphicsRectItem* bottomLeft;
    QGraphicsRectItem* leftCenter;
};
