/*
    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#include "genericchatitemlayout.h"
#include "genericchatitemwidget.h"
#include <QBoxLayout>
#include <cassert>

GenericChatItemLayout::GenericChatItemLayout()
    : layout(new QVBoxLayout())
{
}

GenericChatItemLayout::~GenericChatItemLayout()
{
    delete layout;
}

void GenericChatItemLayout::addSortedWidget(GenericChatItemWidget* widget, int stretch, Qt::Alignment alignment)
{
    int closest = indexOfClosestSortedWidget(widget);
    layout->insertWidget(closest, widget, stretch, alignment);
}

int GenericChatItemLayout::indexOfSortedWidget(GenericChatItemWidget* widget) const
{
    if (layout->count() == 0)
        return -1;

    int index = indexOfClosestSortedWidget(widget);
    if (index >= layout->count())
        return -1;

    GenericChatItemWidget* atMid = dynamic_cast<GenericChatItemWidget*>(layout->itemAt(index)->widget());
    assert(atMid != nullptr);

    if (atMid == widget)
        return index;
    return -1;
}

bool GenericChatItemLayout::existsSortedWidget(GenericChatItemWidget* widget) const
{
    return indexOfSortedWidget(widget) != -1;
}

void GenericChatItemLayout::removeSortedWidget(GenericChatItemWidget* widget)
{
    if (layout->isEmpty())
        return;
    int index = indexOfClosestSortedWidget(widget);

    if (layout->itemAt(index) == nullptr)
        return;

    GenericChatItemWidget* atMid = dynamic_cast<GenericChatItemWidget*>(layout->itemAt(index)->widget());
    assert(atMid != nullptr);

    if (atMid == widget)
        layout->removeWidget(widget);
}

QLayout* GenericChatItemLayout::getLayout() const
{
    return layout;
}

int GenericChatItemLayout::indexOfClosestSortedWidget(GenericChatItemWidget* widget) const
{
    // Binary search: Deferred test of equality.
    int min = 0, max = layout->count(), mid;
    while (min < max)
    {
        mid = (max - min) / 2 + min;
        GenericChatItemWidget* atMid = dynamic_cast<GenericChatItemWidget*>(layout->itemAt(mid)->widget());
        assert(atMid != nullptr);

        if (atMid->getName().localeAwareCompare(widget->getName()) < 0)
            min = mid + 1;
        else
            max = mid;
    }
    return min;
}
