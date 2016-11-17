/*
    Copyright © 2014-2015 by The qTox Project

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

#ifndef GROUPWIDGET_H
#define GROUPWIDGET_H

#include "src/group.h"
#include "src/widget/genericchatroomwidget.h"

class FriendListWidget;

class GroupWidget final : public GenericChatroomWidget
{
    Q_OBJECT
public:
    explicit GroupWidget(Group* group, FriendListWidget* parent);
    ~GroupWidget();

    Type type() const final;
    void setAsActiveChatroom(bool activate) final;
    void updateStatusLight() final;
    bool chatFormIsSet(bool focus) const final;
    void resetEventFlags() final;
    QString getTitle() const final;
    void setName(const QString& name);
    void editName();

    inline Group* getGroup() const
    {
        return g;
    }

signals:
    void groupWidgetClicked(GroupWidget* widget);
    void renameRequested(GroupWidget* widget, const QString& newName);
    void removeGroup(int groupId);

private slots:
    void onUserListChanged(const Group& group, int peerCount, quint8 change);

private:
    void contextMenuEvent(QContextMenuEvent * event) final;
    void mousePressEvent(QMouseEvent* event) final;
    void mouseMoveEvent(QMouseEvent* event) final;
    void dragEnterEvent(QDragEnterEvent* ev) final;
    void dragLeaveEvent(QDragLeaveEvent* ev) final;
    void dropEvent(QDropEvent* ev) final;

private:
    void retranslateUi();

private:
    Group* g;
    bool userWasMentioned;
};

#endif // GROUPWIDGET_H
