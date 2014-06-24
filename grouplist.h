#ifndef GROUPLIST_H
#define GROUPLIST_H

#include <QString>
#include <QList>
#include <QMap>

class Group;

class GroupList
{
public:
    GroupList();
    static Group* addGroup(int groupId, QString name);
    static Group* findGroup(int groupId);
    static void removeGroup(int groupId);

public:
    static QList<Group*> groupList;
};

#endif // GROUPLIST_H
