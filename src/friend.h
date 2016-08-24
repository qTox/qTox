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

class FriendWidget;
class ChatForm;

class Friend : public QObject
{
    Q_OBJECT
public:
    Friend(uint32_t FriendId, const ToxId &UserId);
    Friend(const Friend& other) = delete;
    ~Friend();

    Friend& operator=(const Friend& other) = delete;

    void setName(QString name);
    void setAlias(const QString& name);
    QString getDisplayedName() const;
    bool hasAlias() const;

    void setStatusMessage(const QString& message);
    QString getStatusMessage();

    void setEventFlag(int f);
    int getEventFlag() const;

    const ToxId &getToxId() const;
    uint32_t getFriendId() const;

    void setStatus(Status s);
    Status getStatus() const;

    ChatForm* getChatForm();

signals:
    // TODO: move signals to DB object
    void nameChanged(uint32_t friendId, const QString& name);
    void aliasChanged(uint32_t friendId, const QString& alias);
    void statusChanged(uint32_t friendId, Status status);
    void newStatusMessage(const QString& message);

private:
    QString userName;
    QString userAlias;
    QString statusMessage;
    ToxId userID;
    uint32_t friendId;
    int hasNewEvents;
    Status friendStatus;

    ChatForm* chatForm;
};

#endif // FRIEND_H
