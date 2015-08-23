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

#ifndef DYNAMICSCROLLBAR_H
#define DYNAMICSCROLLBAR_H

#include <QScrollBar>

class DynamicScrollBar : public QScrollBar
{
    Q_OBJECT
public:
    DynamicScrollBar(QWidget *parent = 0);
    DynamicScrollBar(Qt::Orientation orientation, QWidget *parent = 0);

signals:
    void dynamicRequest();

private slots:
    void onSliderMoved(int position);
    void onSliderReleased();

private:
    void init();

    bool ready;
    int lastValue;
};

#endif // DYNAMICSCROLLBAR_H
