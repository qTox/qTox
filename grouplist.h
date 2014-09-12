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

#ifndef GROUPLIST_H
#define GROUPLIST_H

template <typename T>
class QList;
class Group;
class QString;

class GroupList
{
public:
    GroupList();
    static Group* addGroup(int groupId, const QString& name);
    static Group* findGroup(int groupId);
    static void removeGroup(int groupId);

public:
    static QList<Group*> groupList;
};

#endif // GROUPLIST_H
