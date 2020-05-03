/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#include "../chatlinecontent.h"

#include <QLinearGradient>
#include <QPixmap>

class QTimer;

class NotificationIcon : public ChatLineContent
{
    Q_OBJECT
public:
    explicit NotificationIcon(QSize size);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                       QWidget* widget) override;
    void setWidth(qreal width) override;
    qreal getAscent() const override;

private slots:
    void updateGradient();

private:
    QSize size;
    QPixmap pmap;
    QLinearGradient grad;
    QTimer* updateTimer = nullptr;

    qreal dotWidth = 0.2;
    qreal alpha = 0.0;
};
