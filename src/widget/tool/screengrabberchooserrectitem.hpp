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

#ifndef SCREENGRABBERCHOOSERRECTITEM_HPP
#define SCREENGRABBERCHOOSERRECTITEM_HPP

#include <QGraphicsItemGroup>

class ScreenGrabberChooserRectItem : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT
public:
    ScreenGrabberChooserRectItem(QGraphicsScene *scene);
    ~ScreenGrabberChooserRectItem();
    
    QRectF boundingRect() const;
    void beginResize();
    
    QRect chosenRect() const;
    
    void showHandles();
    void hideHandles();
    
signals:
    
    void doubleClicked();
    void regionChosen(QRect rect);
    
protected:
    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);
    
private:
    
    enum State {
        None,
        Resizing,
        HandleResizing,
        Moving,
    };
    
    State state = None;
    int rectWidth = 0;
    int rectHeight = 0;
    
    void forwardMainRectEvent(QEvent *event);
    void forwardHandleEvent(QGraphicsItem *watched, QEvent *event);
    
    void mousePress(QGraphicsSceneMouseEvent *event);
    void mouseMove(QGraphicsSceneMouseEvent *event);
    void mouseRelease(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClick(QGraphicsSceneMouseEvent *event);
    
    void mousePressHandle(int x, int y, QGraphicsSceneMouseEvent *event);
    void mouseMoveHandle(int x, int y, QGraphicsSceneMouseEvent *event);
    void mouseReleaseHandle(int x, int y, QGraphicsSceneMouseEvent *event);
    
    QPoint getHandleMultiplier(QGraphicsItem *handle);
    
    void updateHandlePositions();
    QGraphicsRectItem *createHandleItem(QGraphicsScene *scene);
    
    QGraphicsRectItem *mainRect;
    QGraphicsRectItem *topLeft;
    QGraphicsRectItem *topCenter;
    QGraphicsRectItem *topRight;
    QGraphicsRectItem *rightCenter;
    QGraphicsRectItem *bottomRight;
    QGraphicsRectItem *bottomCenter;
    QGraphicsRectItem *bottomLeft;
    QGraphicsRectItem *leftCenter;
    
};



#endif // SCREENGRABBERCHOOSERRECTITEM_HPP
