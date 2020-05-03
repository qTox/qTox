/*
    Copyright © 2019 by The qTox Project Contributors

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

#pragma once

#include <Qt>

class QLayout;
class QVBoxLayout;
class GenericChatItemWidget;

class GenericChatItemLayout
{
public:
    GenericChatItemLayout();
    GenericChatItemLayout(const GenericChatItemLayout& layout) = delete;
    ~GenericChatItemLayout();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    void addSortedWidget(GenericChatItemWidget* widget, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());
#else
    void addSortedWidget(GenericChatItemWidget* widget, int stretch = 0, Qt::Alignment alignment = nullptr);
#endif
    int indexOfSortedWidget(GenericChatItemWidget* widget) const;
    bool existsSortedWidget(GenericChatItemWidget* widget) const;
    void removeSortedWidget(GenericChatItemWidget* widget);
    void search(const QString& searchString, bool hideAll = false);

    QLayout* getLayout() const;

private:
    int indexOfClosestSortedWidget(GenericChatItemWidget* widget) const;
    QVBoxLayout* layout;
};
