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

class GroupWidget;
class CircleWidget;
class FriendWidget;

class FriendListLayout : public QVBoxLayout
{
    Q_OBJECT
public:
    explicit FriendListLayout(QWidget *parent, bool groupsOnTop = true);

    void addGroupWidget(GroupWidget *widget);
    void addCircleWidget(CircleWidget *widget);
    void addFriendWidget(FriendWidget *widget, Status s);

    virtual void addItem(QLayoutItem *) override;

public:
    QVBoxLayout* getFriendLayout(Status s);

    enum FriendLayoutType
    {
        Online = 0,
        Offline = 1
    };
    QVBoxLayout *friendLayouts[2];
    QVBoxLayout *groupLayout;
    QVBoxLayout *circleLayout;
};

#endif // GENERICFRIENDLISTWIDGET_H
