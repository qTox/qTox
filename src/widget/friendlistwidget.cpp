/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

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
#include "friendlistwidget.h"
#include <QDebug>
#include <QGridLayout>
#include "src/friend.h"
#include "src/friendlist.h"
#include "src/widget/friendwidget.h"

FriendListWidget::FriendListWidget(QWidget *parent, bool groupchatPosition) :
    QWidget(parent)
{
    mainLayout = new QGridLayout();
    setLayout(mainLayout);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    layout()->setSpacing(0);
    layout()->setMargin(0);

    groupLayout = new QVBoxLayout();
    groupLayout->setSpacing(0);
    groupLayout->setMargin(0);

    for (Status s : {Status::Online, Status::Offline})
    {
        QVBoxLayout *l = new QVBoxLayout();
        l->setSpacing(0);
        l->setMargin(0);

        layouts[static_cast<int>(s)] = l;
    }

    if (groupchatPosition)
    {
        mainLayout->addLayout(groupLayout, 0, 0);
        mainLayout->addLayout(layouts[static_cast<int>(Status::Online)], 1, 0);
        mainLayout->addLayout(layouts[static_cast<int>(Status::Offline)], 2, 0);
    }
    else
    {
        mainLayout->addLayout(layouts[static_cast<int>(Status::Online)], 0, 0);
        mainLayout->addLayout(groupLayout, 1, 0);
        mainLayout->addLayout(layouts[static_cast<int>(Status::Offline)], 2, 0);
    }
}

QVBoxLayout* FriendListWidget::getGroupLayout()
{
    return groupLayout;
}

QVBoxLayout* FriendListWidget::getFriendLayout(Status s)
{
    auto res = layouts.find(static_cast<int>(s));
    if (res != layouts.end())
        return res.value();

    //qDebug() << "Friend Status: " << static_cast<int>(s) << " not found!";
    return layouts[static_cast<int>(Status::Online)];
}

void FriendListWidget::onGroupchatPositionChanged(bool top)
{
    mainLayout->removeItem(groupLayout);
    mainLayout->removeItem(getFriendLayout(Status::Online));
    if (top)
    {
        mainLayout->addLayout(groupLayout, 0, 0);
        mainLayout->addLayout(layouts[static_cast<int>(Status::Online)], 1, 0);
    }
    else
    {
        mainLayout->addLayout(layouts[static_cast<int>(Status::Online)], 0, 0);
        mainLayout->addLayout(groupLayout, 1, 0);
    }
}

void FriendListWidget::moveWidget(QWidget *w, Status s)
{
    QVBoxLayout* l = getFriendLayout(s);
    l->removeWidget(w);
    Friend* g = FriendList::findFriend(dynamic_cast<FriendWidget*>(w)->friendId);
    for (int i = 0; i < l->count(); i++)
    {
        FriendWidget* w1 = dynamic_cast<FriendWidget*>(l->itemAt(i)->widget());
        if (w1 != NULL)
        {
            Friend* f = FriendList::findFriend(w1->friendId);
            if (f->getDisplayedName().localeAwareCompare(g->getDisplayedName()) > 0)
            {
                l->insertWidget(i,w);
                return;
            }
        }
    }
    l->addWidget(w);
}
