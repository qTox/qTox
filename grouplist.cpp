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
