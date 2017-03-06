/*
    Copyright © 2014-2015 by The qTox Project Contributors

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

#include "core/toxid.h"
#include "src/core/corestructs.h"
#include <QObject>
#include <QString>

class FriendWidget;
class ChatForm;

class Friend : public QObject
{
    Q_OBJECT
public:
    Friend(uint32_t FriendId, const ToxPk& FriendPk);
    Friend(const Friend& other) = delete;
    ~Friend();
    Friend& operator=(const Friend& other) = delete;

    void loadHistory();

    void setName(QString name);
    void setAlias(QString name);
    QString getDisplayedName() const;
    bool hasAlias() const;

    void setStatusMessage(QString message);
    QString getStatusMessage();

    void setEventFlag(bool f);
    bool getEventFlag() const;

    const ToxPk& getPublicKey() const;
    uint32_t getFriendId() const;

    void setStatus(Status s);
    Status getStatus() const;

    ChatForm* getChatForm();

signals:
    // TODO: move signals to DB object
    void nameChanged(uint32_t friendId, const QString& name);
    void aliasChanged(uint32_t friendId, QString alias);
    void statusChanged(uint32_t friendId, Status status);
    void statusMessageChanged(uint32_t friendId, const QString& message);
    void loadChatHistory();

public slots:

private:
    QString userName;
    QString userAlias;
    QString statusMessage;
    ToxPk friendPk;
    uint32_t friendId;
    bool hasNewEvents;
    Status friendStatus;

    FriendWidget* widget;
    ChatForm* chatForm;
};

#endif // FRIEND_H
