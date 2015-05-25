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

#include "grouplist.h"
#include "group.h"
#include <QHash>
#include <QDebug>

QHash<int, Group*> GroupList::groupList;

Group* GroupList::addGroup(int groupId, const QString& name, bool isAvGroupchat)
{
    auto checker = groupList.find(groupId);
    if (checker != groupList.end())
        qWarning() << "addGroup: groupId already taken";

    Group* newGroup = new Group(groupId, name, isAvGroupchat);
    groupList[groupId] = newGroup;

    return newGroup;
}

Group* GroupList::findGroup(int groupId)
{
    auto g_it = groupList.find(groupId);
    if (g_it != groupList.end())
        return *g_it;

    return nullptr;
}

void GroupList::removeGroup(int groupId, bool /*fake*/)
{
    auto g_it = groupList.find(groupId);
    if (g_it != groupList.end())
    {
        groupList.erase(g_it);
    }
}

QList<Group *> GroupList::getAllGroups()
{
    QList<Group*> res;

    for (auto it : groupList)
        res.append(it);

    return res;
}

void GroupList::clear()
{
    for (auto groupptr : groupList)
        delete groupptr;
    groupList.clear();
}
