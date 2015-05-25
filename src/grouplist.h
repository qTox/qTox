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

#ifndef GROUPLIST_H
#define GROUPLIST_H

template <class A, class B> class QHash;
template <class T> class QList;
class Group;
class QString;

class GroupList
{
public:
    static Group* addGroup(int groupId, const QString& name, bool isAvGroupchat);
    static Group* findGroup(int groupId);
    static void removeGroup(int groupId, bool fake = false);
    static QList<Group*> getAllGroups();
    static void clear();

private:
    static QHash<int, Group*> groupList;
};

#endif // GROUPLIST_H
