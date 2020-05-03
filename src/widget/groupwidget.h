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

#pragma once

#include "genericchatroomwidget.h"

#include "src/model/chatroom/groupchatroom.h"
#include "src/core/groupid.h"

#include <memory>

class GroupWidget final : public GenericChatroomWidget
{
    Q_OBJECT
public:
    GroupWidget(std::shared_ptr<GroupChatroom> chatroom, bool compact);
    ~GroupWidget();
   void setAsInactiveChatroom() final;
    void setAsActiveChatroom() final;
    void updateStatusLight() final;
    void resetEventFlags() final;
    QString getStatusString() const final;
    Group* getGroup() const final;
    const Contact* getContact() const final;
    void setName(const QString& name);
    void editName();

signals:
    void groupWidgetClicked(GroupWidget* widget);
    void removeGroup(const GroupId& groupId);

protected:
    void contextMenuEvent(QContextMenuEvent* event) final;
    void mousePressEvent(QMouseEvent* event) final;
    void mouseMoveEvent(QMouseEvent* event) final;
    void dragEnterEvent(QDragEnterEvent* ev) override;
    void dragLeaveEvent(QDragLeaveEvent* ev) override;
    void dropEvent(QDropEvent* ev) override;

private slots:
    void retranslateUi();
    void updateTitle(const QString& author, const QString& newName);
    void updateUserCount(int numPeers);

public:
    GroupId groupId;

private:
    std::shared_ptr<GroupChatroom> chatroom;
};
