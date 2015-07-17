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

#include "dynamicscrollbar.h"

DynamicScrollBar::DynamicScrollBar(QWidget *parent)
    : QScrollBar(parent)
{
    init();
}

DynamicScrollBar::DynamicScrollBar(Qt::Orientation orientation, QWidget *parent)
    : QScrollBar(orientation, parent)
{
    init();
}

void DynamicScrollBar::onSliderMoved(int position)
{
    if (position == minimum() && lastValue != position)
    {
        if (!isSliderDown())
            emit dynamicRequest();
        else
            ready = true;
    }
    else
    {
        ready = false;
    }

    lastValue = position;
}

void DynamicScrollBar::onSliderReleased()
{
    if (ready)
    {
        emit dynamicRequest();
        ready = false;
    }
}

void DynamicScrollBar::init()
{
    connect(this, &QScrollBar::valueChanged, this, &DynamicScrollBar::onSliderMoved);
    connect(this, &QScrollBar::sliderReleased, this, &DynamicScrollBar::onSliderReleased);
}
