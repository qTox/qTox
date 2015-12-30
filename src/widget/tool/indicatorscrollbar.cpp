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

#include "indicatorscrollbar.h"
#include <QPainter>
#include <QStyleOptionSlider>

IndicatorScrollBar::IndicatorScrollBar(int total, QWidget *parent)
    : DynamicScrollBar(parent)
{
    setTotal(total);
}

void IndicatorScrollBar::setTotal(int total)
{
    this->total = total;
}

void IndicatorScrollBar::addIndicator(int pos)
{
    indicators.push_back(pos);
    update();
}

void IndicatorScrollBar::clearIndicators()
{
    indicators.clear();
    indicators.squeeze();
    update();
}

void IndicatorScrollBar::paintEvent(QPaintEvent *event)
{
    QScrollBar::paintEvent(event);

    if (!indicators.isEmpty())
    {
        QPainter painter(this);
        painter.setBrush(Qt::yellow);
        painter.setPen(Qt::darkYellow);

        for (int pos : indicators)
        {
            int loc = (static_cast<float>(pos) / total) * height();
            painter.drawRect(0, loc - 1, width() - 1, 2);
        }
    }
}

