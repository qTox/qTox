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
#ifndef GENERICCHATITEMLAYOUT_H
#define GENERICCHATITEMLAYOUT_H

#include <Qt>

class QLayout;
class QVBoxLayout;
class GenericChatItemWidget;

class GenericChatItemLayout
{
public:

    GenericChatItemLayout();
    ~GenericChatItemLayout();

    void addSortedWidget(GenericChatItemWidget* widget, int stretch = 0, Qt::Alignment alignment = 0);
    int indexOfSortedWidget(GenericChatItemWidget* widget) const;
    bool existsSortedWidget(GenericChatItemWidget* widget) const;
    void removeSortedWidget(GenericChatItemWidget* widget);
    void search(const QString &searchString, bool hideAll = false);

    QLayout* getLayout() const;

private:
    int indexOfClosestSortedWidget(GenericChatItemWidget* widget) const;
    QVBoxLayout* layout;
};

#endif // GENERICCHATITEMLAYOUT_H
