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

#include "spinner.h"
#include "../pixmapcache.h"

#include <QDebug>
#include <QGraphicsScene>
#include <QPainter>
#include <QTime>
#include <QVariantAnimation>

#include <math.h>

Spinner::Spinner(const QString& img, QSize Size, qreal speed)
    : size(Size)
    , rotSpeed(speed)
{
    pmap = PixmapCache::getInstance().get(img, size);

    // Timer for the animation, if the Widget is not redrawn, no paint events will
    // arrive and the timer will not be restarted, so this stops automatically
    timer.setInterval(1000 / framerate);
    timer.setSingleShot(true);

    blendAnimation = new QVariantAnimation(this);
    blendAnimation->setStartValue(0.0);
    blendAnimation->setEndValue(1.0);
    blendAnimation->setDuration(350);
    blendAnimation->setEasingCurve(QEasingCurve::InCubic);
    blendAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    connect(blendAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant& val) { alpha = val.toDouble(); });

    QObject::connect(&timer, &QTimer::timeout, this, &Spinner::timeout);
}

QRectF Spinner::boundingRect() const
{
    return QRectF(QPointF(-size.width() / 2.0, -size.height() / 2.0), size);
}

void Spinner::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->setClipRect(boundingRect());

    QTransform trans = QTransform().rotate(static_cast<qreal>(curRot))
                           .translate(-size.width() / 2.0, -size.height() / 2.0);
    painter->setOpacity(alpha);
    painter->setTransform(trans, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawPixmap(0, 0, pmap);

    if (!timer.isActive()) {
        timer.start(); // update bounding rectangle for next frame
    }

    std::ignore = option;
    std::ignore = widget;
}

void Spinner::setWidth(float width)
{
    std::ignore = width;
}

qreal Spinner::getAscent() const
{
    return 0.0;
}

void Spinner::timeout()
{
    // Use global time, so the animations are synced
    float angle = QTime::currentTime().msecsSinceStartOfDay() / 1000.0f * rotSpeed;
    // limit to the range [0.0 - 360.0]
    curRot = remainderf(angle, 360.0f);

    if (scene()) {
        scene()->invalidate(sceneBoundingRect());
    }
}
