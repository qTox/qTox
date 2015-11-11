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

#include "micfeedbackwidget.h"
#include "src/audio/audio.h"
#include <QPainter>
#include <QLinearGradient>

MicFeedbackWidget::MicFeedbackWidget(QWidget *parent)
    : QWidget(parent)
    , timerId(0)
{
    setFixedHeight(20);
}

void MicFeedbackWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setPen(QPen(Qt::black));
    painter.drawRect(QRect(0, 0, width() - 1, height() - 1));

    int gradientWidth = round(width() * current) - 4;

    if (gradientWidth < 0)
        gradientWidth = 0;

    QRect gradientRect(2, 2, gradientWidth, height() - 4);

    QLinearGradient gradient(0, 0, width(), 0);
    gradient.setColorAt(0, Qt::green);
    gradient.setColorAt(0.5, Qt::yellow);
    gradient.setColorAt(1, Qt::red);
    painter.fillRect(gradientRect, gradient);

    float slice = width() / 5;
    int padding = slice / 2;

    for (int i = 0; i < 5; ++i)
    {
        float pos = slice * i + padding;
        painter.drawLine(pos, 2, pos, height() - 4);
    }
}

void MicFeedbackWidget::timerEvent(QTimerEvent*)
{
    const int framesize = AUDIO_FRAME_SAMPLE_COUNT * AUDIO_CHANNELS;
    int16_t buff[framesize] = {0};

    if (Audio::getInstance().tryCaptureSamples(buff, AUDIO_FRAME_SAMPLE_COUNT))
    {
        double max = 0;

        for (int i = 0; i < framesize; ++i)
            max = std::max(max, fabs(buff[i] / 32767.0));

        if (max > current)
            current = max;
        else
            current -= 0.05;

        update();
    }
    else if (current > 0)
    {
        current -= 0.01;
    }
}

void MicFeedbackWidget::showEvent(QShowEvent*)
{
    timerId = startTimer(60);
}

void MicFeedbackWidget::hideEvent(QHideEvent*)
{
    if (timerId != 0)
    {
        killTimer(timerId);
        timerId = 0;
    }
}
