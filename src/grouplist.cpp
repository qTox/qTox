/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#include "grouplist.h"
#include "src/core/core.h"
#include "src/model/group.h"
#include <QDebug>
#include <QHash>

QHash<const GroupId, Group*> GroupList::groupList;
QHash<uint32_t, GroupId> GroupList::id2key;
Group* GroupList::addGroup(Core& core, int groupNum, const GroupId& groupId, const QString& name, bool isAvGroupchat,
                           const QString& selfName)
{
    auto checker = groupList.find(groupId);
    if (checker != groupList.end()) {
        qWarning() << "addGroup: groupId already taken";
    }

    Group* newGroup = new Group(groupNum, groupId, name, isAvGroupchat, selfName, core, core);
    groupList[groupId] = newGroup;
    id2key[groupNum] = groupId;
    return newGroup;
}

Group* GroupList::findGroup(const GroupId& groupId)
{
    auto g_it = groupList.find(groupId);
    if (g_it != groupList.end())
        return *g_it;

    return nullptr;
}

const GroupId& GroupList::id2Key(uint32_t groupNum)
{
    return id2key[groupNum];
}

void GroupList::removeGroup(const GroupId& groupId, bool /*fake*/)
{
    auto g_it = groupList.find(groupId);
    if (g_it != groupList.end()) {
        groupList.erase(g_it);
    }
}

QList<Group*> GroupList::getAllGroups()
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
