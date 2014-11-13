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

#ifndef GROUPCHATFORM_H
#define GROUPCHATFORM_H

#include "genericchatform.h"

namespace Ui {class MainWindow;}
class Group;
class TabCompleter;
class FlowLayout;

class GroupChatForm : public GenericChatForm
{
    Q_OBJECT
public:
    GroupChatForm(Group* chatGroup);

    void onUserListChanged();

signals:
    void groupTitleChanged(int groupnum, const QString& name);

private slots:
    void onSendTriggered();

protected:
    // drag & drop
    void dragEnterEvent(QDragEnterEvent* ev);
    void dropEvent(QDropEvent* ev);

private:
    Group* group;
    FlowLayout* namesListLayout;
    QLabel *nusersLabel;
    TabCompleter* tabber;
};

#endif // GROUPCHATFORM_H
