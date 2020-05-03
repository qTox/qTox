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

#include <QWidget>

class MovableWidget : public QWidget
{
public:
    explicit MovableWidget(QWidget* parent);
    void resetBoundary(QRect newBoundary);
    void setBoundary(QRect newBoundary);
    float getRatio() const;
    void setRatio(float r);

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);

private:
    void checkBoundary(QPoint& point) const;
    void checkBoundaryLeft(int& x) const;

    typedef uint8_t Modes;

    enum Mode : Modes
    {
        Moving = 0x01,
        ResizeLeft = 0x02,
        ResizeRight = 0x04,
        ResizeUp = 0x08,
        ResizeDown = 0x10,
        Resize = ResizeLeft | ResizeRight | ResizeUp | ResizeDown
    };

    Modes mode = 0;
    QPoint lastPoint;
    QRect boundaryRect;
    QSizeF actualSize;
    QPointF actualPos;
    float ratio;
};
