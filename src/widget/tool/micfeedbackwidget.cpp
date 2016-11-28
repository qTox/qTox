/*
    Copyright © 2015 by The qTox Project Contributors

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

#include "micfeedbackwidget.h"
#include "src/audio/audio.h"
#include <QPainter>
#include <QLinearGradient>

MicFeedbackWidget::MicFeedbackWidget(QWidget *parent)
    : QWidget(parent), mMeterListener(nullptr)
{
    setFixedHeight(20);
}

void MicFeedbackWidget::paintEvent(QPaintEvent*)
{
    const int w = width();
    const int h = height();
    QPainter painter(this);
    painter.setPen(QPen(Qt::gray));
    painter.drawRoundedRect(QRect(0, 0, w - 1, h - 1), 3., 3.);

    int gradientWidth = qMax(0, qRound(w * current) - 4);

    QRect gradientRect(2, 2, gradientWidth, h - 4);

    QPainterPath path;
    QLinearGradient gradient(0, 0, w, 0);
    gradient.setColorAt(0.0, Qt::green);
    gradient.setColorAt(0.5, Qt::yellow);
    gradient.setColorAt(1.0, Qt::red);
    path.addRoundedRect(gradientRect, 2.0, 2.0);
    painter.fillPath(path, gradient);

    const float slice = w / 5.f;
    const int padding = qRound(slice / 2);

    for (int i = 0; i < 5; ++i)
    {
        int pos = qRound(slice * i + padding);
        painter.drawLine(pos, 2, pos, h - 4);
    }
}

void MicFeedbackWidget::showEvent(QShowEvent*)
{
#if 0
    mMeterListener = Audio::getInstance().createAudioMeterListener();
    connect(mMeterListener, &AudioMeterListener::gainChanged,
            this, &MicFeedbackWidget::onGainMetered);
    mMeterListener->start();
#endif
}

void MicFeedbackWidget::hideEvent(QHideEvent*)
{
#if 0
    mMeterListener->stop();
#endif
}

void MicFeedbackWidget::onGainMetered(qreal value)
{
    Q_UNUSED(value);
#if 0
    current = value;
    update();
    mMeterListener->processed();
#endif
}
