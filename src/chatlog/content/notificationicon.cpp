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

#include "notificationicon.h"
#include "../pixmapcache.h"
#include "src/widget/style.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QTimer>

NotificationIcon::NotificationIcon(QSize Size)
    : size(Size)
{
    pmap = PixmapCache::getInstance().get(Style::getImagePath("chatArea/typing.svg"), size);

    updateTimer = new QTimer(this);
    updateTimer->setInterval(1000 / 30);
    updateTimer->setSingleShot(false);

    updateTimer->start();

    connect(updateTimer, &QTimer::timeout, this, &NotificationIcon::updateGradient);
}

QRectF NotificationIcon::boundingRect() const
{
    return QRectF(QPointF(-size.width() / 2.0, -size.height() / 2.0), size);
}

void NotificationIcon::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->setClipRect(boundingRect());

    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->translate(-size.width() / 2.0, -size.height() / 2.0);

    painter->fillRect(QRect(0, 0, size.width(), size.height()), grad);
    painter->drawPixmap(0, 0, size.width(), size.height(), pmap);

    Q_UNUSED(option)
    Q_UNUSED(widget)
}

void NotificationIcon::setWidth(qreal width)
{
    Q_UNUSED(width)
}

qreal NotificationIcon::getAscent() const
{
    return 3.0;
}

void NotificationIcon::updateGradient()
{
    alpha += 0.01;

    if (alpha + dotWidth >= 1.0)
        alpha = 0.0;

    grad = QLinearGradient(QPointF(-0.5 * size.width(), 0), QPointF(3.0 / 2.0 * size.width(), 0));
    grad.setColorAt(0, Qt::lightGray);
    grad.setColorAt(qMax(0.0, alpha - dotWidth), Qt::lightGray);
    grad.setColorAt(alpha, Qt::black);
    grad.setColorAt(qMin(1.0, alpha + dotWidth), Qt::lightGray);
    grad.setColorAt(1, Qt::lightGray);

    if (scene() && isVisible())
        scene()->invalidate(sceneBoundingRect());
}
