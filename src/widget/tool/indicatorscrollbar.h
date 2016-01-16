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

#ifndef INDICATORSCROLLBAR_H
#define INDICATORSCROLLBAR_H

#include <QScrollBar>

class IndicatorScrollBar : public QScrollBar
{
public:
    IndicatorScrollBar(int total, QWidget *parent = 0);
    void setTotal(int total);
    void addIndicator(int pos);
    void clearIndicators();

protected:
    virtual void paintEvent(QPaintEvent *event) final override;

private:
    QVector<int> indicators;
    int total;
};

#endif // INDICATORSCROLLBAR_H
