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

#include "aspectratiowidget.h"
#include <QApplication>

AspectRatioWidget::AspectRatioWidget(QWidget *parent)
    : QWidget(parent)
    , ratioMode{RespectWidth}
    , ratioWidth{QWIDGETSIZE_MAX}
{
    setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
    setRatio(1.33f);
    //setSizeHint(64, 64 / 1.33f);
    size_ = QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}

float AspectRatioWidget::getRatio() const
{
    return ratio;
}

bool AspectRatioWidget::aspectRatioRespected() const
{
    int w = static_cast<int>(width() / ratio);
    return height() == w - 1 || height() == w || height() == w + 1;
}
#include <QDebug>
QSize AspectRatioWidget::sizeHint() const
{
    //return QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    return size_;
    switch (ratioMode)
    {
        case RespectWidth:
            return QSize(ratioWidth, width() / ratio);
        case RespectHeight:
            return QSize(height() * ratio, height());
        case MaximizeSize:
        default:
            return QSize(ratioWidth, ratioWidth);
    }
}
#include <QDebug>
void AspectRatioWidget::setSizeHint(int width, int height)
{
    size_ = QSize(width, height);
}

void AspectRatioWidget::setMinimum(int minimumHieght)
{
    setMinimumSize(minimumHieght * ratio, minimumHieght);
}

void AspectRatioWidget::setRatio(float r)
{
    if (ratio == r)
        return;

    ratio = r;
    setMinimum(minimumHeight());
}

void AspectRatioWidget::setRatioMode(RatioMode mode)
{
    ratioMode = mode;

    if (sizeHint().width() > 1)
        setMaximumWidth(sizeHint().width());

    updateGeometry();
}

void AspectRatioWidget::setRatioWidth(int w)
{
    ratioWidth = w;
}

void AspectRatioWidgetList::updateAll()
{
    if (isEmpty())
        return;

    // Maximize the size of all of them, so that they will be equal.
    for (AspectRatioWidget* aspectWidget : *this)
    {
        aspectWidget->setUpdatesEnabled(false);
        aspectWidget->setRatioMode(AspectRatioWidget::MaximizeSize);
    }

    // Need this to trigger layout recalculations.
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers);

    // Now update heights.
    for (AspectRatioWidget* aspectWidget : *this)
        aspectWidget->setRatioMode(AspectRatioWidget::RespectWidth);

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    // if height does not fit, then decrease width.
    // Query only one. Assumes that all of them are next to each other.
    if (!at(0)->aspectRatioRespected())
    {
        for (AspectRatioWidget* aspectWidget : *this)
            aspectWidget->setRatioMode(AspectRatioWidget::RespectHeight);

        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    for (AspectRatioWidget* aspectWidget : *this)
    {
        aspectWidget->setUpdatesEnabled(true);
        aspectWidget->update();
    }
}
