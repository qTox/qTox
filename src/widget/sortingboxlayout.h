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
#ifndef SORTINGBOXLAYOUT_H
#define SORTINGBOXLAYOUT_H

#include <QBoxLayout>
#include <cassert>

#include <QDebug>

template <typename T, QBoxLayout::Direction Dir>
class SortingBoxLayout
{
public:

    static_assert(std::is_base_of<QWidget,T>::value == true, "T must be base of QWidget*.");

    SortingBoxLayout();
    ~SortingBoxLayout();

    void addSortedWidget(T* widget);
    int indexOfSortedWidget(T* widget) const;
    bool existsSortedWidget(T* widget) const;
    void removeSortedWidget(T* widget);

    QLayout* getLayout() const;

private:
    int indexOfClosestSortedWidget(T* widget) const;
    QBoxLayout* layout;
};

template <typename T, QBoxLayout::Direction Dir>
SortingBoxLayout<T, Dir>::SortingBoxLayout()
    : layout(new QBoxLayout(Dir))
{
}

template <typename T, QBoxLayout::Direction Dir>
SortingBoxLayout<T, Dir>::~SortingBoxLayout()
{
    delete layout;
}

template <typename T, QBoxLayout::Direction Dir>
void SortingBoxLayout<T, Dir>::addSortedWidget(T* widget)
{
    int closest = indexOfClosestSortedWidget(widget);
    qDebug() << closest;
    layout->insertWidget(closest, widget);
}

template <typename T, QBoxLayout::Direction Dir>
int SortingBoxLayout<T, Dir>::indexOfSortedWidget(T* widget) const
{
    if (layout->count() == 0)
        return -1;

    int index = indexOfClosestSortedWidget(widget);
    if (index >= layout->count())
        return -1;

    T* atMid = dynamic_cast<T*>(layout->itemAt(index)->widget());
    assert(atMid != nullptr);

    if (atMid == widget)
        return index;
    return -1;
}

template <typename T, QBoxLayout::Direction Dir>
bool SortingBoxLayout<T, Dir>::existsSortedWidget(T* widget) const
{
    return indexOfSortedWidget(widget) != -1;
}

template <typename T, QBoxLayout::Direction Dir>
void SortingBoxLayout<T, Dir>::removeSortedWidget(T* widget)
{
    if (layout->isEmpty())
        return;
    int index = indexOfClosestSortedWidget(widget);

    if (layout->itemAt(index) == nullptr)
        return;

    T* atMid = dynamic_cast<T*>(layout->itemAt(index)->widget());
    assert(atMid != nullptr);

    if (atMid == widget)
        layout->removeWidget(widget);
}

template <typename T, QBoxLayout::Direction Dir>
QLayout* SortingBoxLayout<T, Dir>::getLayout() const
{
    return layout;
}

template <typename T, QBoxLayout::Direction Dir>
int SortingBoxLayout<T, Dir>::indexOfClosestSortedWidget(T* widget) const
{
    // Binary search: Deferred test of equality.
    int min = 0, max = layout->count(), mid;
    while (min < max)
    {
        qDebug() << "min";
        mid = (max - min) / 2 + min;
        T* atMid = dynamic_cast<T*>(layout->itemAt(mid)->widget());
        assert(atMid != nullptr);

        if (*widget < *atMid)
            min = mid + 1;
        else
            max = mid;
    }
    return min;
}


template <typename T>
using VSortingBoxLayout = SortingBoxLayout<T, QBoxLayout::TopToBottom>;

#endif // SORTINGBOXLAYOUT_H
