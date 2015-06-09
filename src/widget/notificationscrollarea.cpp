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
#include "notificationscrollarea.h"
#include "notificationedgewidget.h"
#include "genericchatroomwidget.h"
#include <QScrollBar>
#include <cassert>

NotificationScrollArea::NotificationScrollArea(QWidget* parent)
    : AdjustingScrollArea(parent)
{
    connect(verticalScrollBar(), &QAbstractSlider::valueChanged, this, &NotificationScrollArea::updateTracking);
    connect(verticalScrollBar(), &QAbstractSlider::rangeChanged, this, &NotificationScrollArea::updateTracking);
}

void NotificationScrollArea::trackWidget(GenericChatroomWidget* widget)
{
    if (trackedWidgets.find(widget) != trackedWidgets.end())
        return;

    Visibility visibility = widgetVisible(widget);
    if (visibility != Visible)
    {
        if (visibility == Above)
        {
            if (referencesAbove++ == 0)
            {
                assert(topEdge == nullptr);
                topEdge = new NotificationEdgeWidget(NotificationEdgeWidget::Top, this);
                connect(topEdge, &NotificationEdgeWidget::clicked, this, &NotificationScrollArea::findPreviousWidget);
                recalculateTopEdge();
                topEdge->show();
            }
        }
        else
        {
            if (referencesBelow++ == 0)
            {
                assert(bottomEdge == nullptr);
                bottomEdge = new NotificationEdgeWidget(NotificationEdgeWidget::Bottom, this);
                connect(bottomEdge, &NotificationEdgeWidget::clicked, this, &NotificationScrollArea::findNextWidget);
                recalculateBottomEdge();
                bottomEdge->show();
            }
        }

        trackedWidgets.insert(widget, visibility);
    }
}

void NotificationScrollArea::updateTracking()
{
    QHash<GenericChatroomWidget*, Visibility>::iterator i = trackedWidgets.begin();
    while (i != trackedWidgets.end())
    {
        if (widgetVisible(i.key()) == Visible)
        {
            if (i.value() == Above)
            {
                if (--referencesAbove == 0)
                {
                    delete topEdge;
                    topEdge = nullptr;
                }
            }
            else
            {
                if (--referencesBelow == 0)
                {
                    delete bottomEdge;
                    bottomEdge = nullptr;
                }
            }
            i = trackedWidgets.erase(i);
            continue;
        }
        ++i;
    }
}

void NotificationScrollArea::resizeEvent(QResizeEvent *event)
{
    if (topEdge != nullptr)
        recalculateTopEdge();
    if (bottomEdge != nullptr)
        recalculateBottomEdge();

    AdjustingScrollArea::resizeEvent(event);
}

void NotificationScrollArea::findNextWidget()
{
    GenericChatroomWidget* next = nullptr;
    int value;
    QHash<GenericChatroomWidget*, Visibility>::iterator i = trackedWidgets.begin();

    // Find the first next, to avoid nullptr.
    for (; i != trackedWidgets.end(); ++i)
    {
        if (i.value() == Below)
        {
            next = i.key();
            value = next->mapTo(viewport(), QPoint()).y();
            break;
        }
    }

    // Try finding a closer one.
    for (; i != trackedWidgets.end(); ++i)
    {
        if (i.value() == Below)
        {
            int y = i.key()->mapTo(viewport(), QPoint()).y();
            if (y < value)
            {
                next = i.key();
                value = y;
            }
        }
    }

    if (next != nullptr)
        ensureWidgetVisible(next, 0, referencesBelow != 1 ? bottomEdge->height() : 0);
}

void NotificationScrollArea::findPreviousWidget()
{
    GenericChatroomWidget* next = nullptr;
    int value;
    QHash<GenericChatroomWidget*, Visibility>::iterator i = trackedWidgets.begin();

    // Find the first next, to avoid nullptr.
    for (; i != trackedWidgets.end(); ++i)
    {
        if (i.value() == Above)
        {
            next = i.key();
            value = next->mapTo(viewport(), QPoint()).y();
            break;
        }
    }

    // Try finding a closer one.
    for (; i != trackedWidgets.end(); ++i)
    {
        if (i.value() == Above)
        {
            int y = i.key()->mapTo(viewport(), QPoint()).y();
            if (y > value)
            {
                next = i.key();
                value = y;
            }
        }
    }

    if (next != nullptr)
        ensureWidgetVisible(next, 0, referencesAbove != 1 ? topEdge->height() : 0);
}

NotificationScrollArea::Visibility NotificationScrollArea::widgetVisible(QWidget *widget) const
{
    int y = widget->mapTo(viewport(), QPoint()).y();

    if (y < 0)
        return Above;
    else if (y + widget->height() - 1 > viewport()->height())
        return Below;

    return Visible;
}

void NotificationScrollArea::recalculateTopEdge()
{
    topEdge->move(viewport()->pos());
    topEdge->resize(viewport()->width(), topEdge->height());
}

void NotificationScrollArea::recalculateBottomEdge()
{
    QPoint position = viewport()->pos();
    position.setY(position.y() + viewport()->height() - bottomEdge->height());
    bottomEdge->move(position);
    bottomEdge->resize(viewport()->width(), bottomEdge->height());
}
