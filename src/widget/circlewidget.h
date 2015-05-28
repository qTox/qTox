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

#ifndef CIRCLEWIDGET_H
#define CIRCLEWIDGET_H

#include "genericchatitemwidget.h"
#include "src/core/corestructs.h"

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class FriendListLayout;
class FriendWidget;

class CircleWidget : public GenericChatItemWidget
{
    Q_OBJECT
public:
    CircleWidget(QWidget *parent = 0);

    void addFriendWidget(FriendWidget *w, Status s);

    void searchChatrooms(const QString &searchString, bool hideOnline = false, bool hideOffline = false, bool hideGroups = false);

    void toggle();

protected:

    void mousePressEvent(QMouseEvent *event) override;

    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent* event) override;

private:
    enum FriendLayoutType
    {
        Online = 0,
        Offline = 1
    };
    bool visible = false;
    FriendListLayout *listLayout;
    QVBoxLayout *mainLayout;
    QLabel *arrowLabel;
    QWidget *container;
};

#endif // CIRCLEWIDGET_H
