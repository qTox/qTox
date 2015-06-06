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

#ifndef FRIEND_H
#define FRIEND_H

#include <QObject>
#include <QString>
#include "src/core/corestructs.h"
#include "core/toxid.h"

struct FriendWidget;
class ChatForm;

class Friend : public QObject
{
    Q_OBJECT
public:
    Friend(uint32_t FriendId, const ToxId &UserId);
    Friend(const Friend& other)=delete;
    ~Friend();
    Friend& operator=(const Friend& other)=delete;

    /// Loads the friend's chat history if enabled
    void loadHistory();

    void setName(QString name);
    void setAlias(QString name);
    QString getDisplayedName() const;

    void setStatusMessage(QString message);

    void setEventFlag(int f);
    int getEventFlag() const;

    const ToxId &getToxId() const;
    uint32_t getFriendID() const;

    void setStatus(Status s);
    Status getStatus() const;

    ChatForm *getChatForm();
    FriendWidget *getFriendWidget();

signals:
    void displayedNameChanged(FriendWidget* widget, Status s, int hasNewEvents);

private:
    QString userAlias, userName;
    ToxId userID;
    uint32_t friendId;
    int hasNewEvents;
    Status friendStatus;

    FriendWidget* widget;
    ChatForm* chatForm;
};

#endif // FRIEND_H
