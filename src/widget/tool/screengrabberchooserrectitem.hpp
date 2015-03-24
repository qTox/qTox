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

#include <QGraphicsObject>

class ScreenGrabberChooserRectItem : public QGraphicsObject
{
    Q_OBJECT
public:
    ScreenGrabberChooserRectItem();
    ~ScreenGrabberChooserRectItem();
    
    int width() const;
    int height() const;
    
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void beginResize();
    
    QRect chosenRect() const;
    
signals:
    
    void doubleClicked();
    void regionChosen(QRect rect);
    
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    
private:
    
    enum State {
        None,
        Resizing,
        Moving,
    };
    
    State state = None;
    int rectWidth = 0;
    int rectHeight = 0;
    
};



#endif // SCREENGRABBERCHOOSERRECTITEM_HPP
