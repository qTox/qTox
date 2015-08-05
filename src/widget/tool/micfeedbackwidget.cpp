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

MicFeedbackWidget::MicFeedbackWidget(QWidget *parent) : QWidget(parent)
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
    gradient.setColorAt(0, Qt::red);
    gradient.setColorAt(0.5, Qt::yellow);
    gradient.setColorAt(1, Qt::green);
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
    const int framesize = (20 * 48000) / 1000;
    const int bufsize = framesize * 2;
    uint8_t buff[bufsize];
    memset(buff, 0, bufsize);

    if (Audio::tryCaptureSamples(buff, framesize))
    {
        double max = 0;
        int16_t* buffReal = reinterpret_cast<int16_t*>(&buff[0]);

        for (int i = 0; i < bufsize / 2; ++i)
            max = std::max(max, fabs(buffReal[i] / 32767.0));

        if (max > current)
            current = max;
        else
            current -= 0.05;

        update();
    }
}

void MicFeedbackWidget::showEvent(QShowEvent*)
{
    Audio::suscribeInput();
    int timerId = startTimer(60);
}

void MicFeedbackWidget::hideEvent(QHideEvent*)
{
    Audio::unsuscribeInput();
    killTimer(timerId);
}
