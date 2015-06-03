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

#ifndef GENERICFRIENDLISTWIDGET_H
#define GENERICFRIENDLISTWIDGET_H

#include <QBoxLayout>
#include "src/core/corestructs.h"
#include "sortingboxlayout.h"
#include "friendwidget.h"

class GroupWidget;
class CircleWidget;
class FriendWidget;
class FriendListWidget;

class FriendListLayout : public QVBoxLayout
{
    Q_OBJECT
public:
    explicit FriendListLayout();

    void addFriendWidget(FriendWidget* widget, Status s);
    int indexOfFriendWidget(FriendWidget* widget, bool online) const;
    void moveFriendWidgets(FriendListWidget* listWidget);
    int friendOnlineCount() const;
    int friendTotalCount() const;

    bool hasChatrooms() const;
    void searchChatrooms(const QString& searchString, bool hideOnline = false, bool hideOffline = false);

    template <typename WidgetType>
    static void searchLayout(const QString& searchString, QLayout* boxLayout, bool hideAll);

    QLayout* getLayoutOnline() const;
    QLayout* getLayoutOffline() const;

private:
    QLayout* getFriendLayout(Status s);

    VSortingBoxLayout<FriendWidget> friendOnlineLayout;
    VSortingBoxLayout<FriendWidget> friendOfflineLayout;
};

template <typename WidgetType>
void FriendListLayout::searchLayout(const QString &searchString, QLayout *boxLayout, bool hideAll)
{
    for (int index = 0; index < boxLayout->count(); ++index)
    {
        WidgetType* widgetAt = static_cast<WidgetType*>(boxLayout->itemAt(index)->widget());
        QString widgetName = widgetAt->getName();

        widgetAt->setVisible(!hideAll && widgetName.contains(searchString, Qt::CaseInsensitive));
    }
}

#endif // GENERICFRIENDLISTWIDGET_H
