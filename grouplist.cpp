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

#include "grouplist.h"
#include "group.h"

QList<Group*> GroupList::groupList;

Group* GroupList::addGroup(int groupId, QString name)
{
    Group* newGroup = new Group(groupId, name);
    groupList.append(newGroup);
    return newGroup;
}

Group* GroupList::findGroup(int groupId)
{
    for (Group* g : groupList)
        if (g->groupId == groupId)
            return g;
    return nullptr;
}

void GroupList::removeGroup(int groupId)
{
    for (int i=0; i<groupList.size(); i++)
    {
        if (groupList[i]->groupId == groupId)
        {
            groupList.removeAt(i);
            return;
        }
    }
}
