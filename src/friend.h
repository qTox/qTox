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

#include <cassert>

#include <QObject>
#include <QPixmap>
#include <QString>

#include "src/chatlog/chatmessage.h"
#include "src/core/corestructs.h"
#include "src/core/toxid.h"
#include "src/core/cstring.h"
#include "src/persistence/offlinemsgengine.h"

class FriendNotify;
struct Tox;

class Friend final
{
    friend class CoreFile;
    friend class FriendNotify;

    class Private;
    using PrivatePtr = QExplicitlySharedDataPointer<Private>;

public:
    using ID = uint32_t;
    using FriendCache = QHash<ID, Private*>;
    using List = QList<Friend>;
    using IDList = QVector<ID>;

public:
    inline static const FriendNotify* notify()
    {
        return &notifier;
    }

    inline static QPixmap getDefaultAvatar(bool active = true)
    {
        return active ? QPixmap(QStringLiteral(":/img/contact_dark.svg"))
                      : QPixmap(QStringLiteral(":/img/contact.svg"));
    }

    inline static QString statusToString(const Friend& f)
    {
        if (f)
        {
            switch (f.getStatus())
            {
            case Status::Online:
                return QObject::tr("Online");
            case Status::Away:
                return QObject::tr("Away");
            case Status::Busy:
                return QObject::tr("Busy");
            case Status::Offline:
                return QObject::tr("Offline");
            }
        }

        return QString();
    }

    inline static void toOneline(QString& str)
    {
        str.replace('\n', ' ');
        str.remove('\r');
    }

    inline static Friend get(const ToxId& userId)
    {
        auto it = tox2id.find(userId.publicKey);
        return it == tox2id.end() ? nullptr : get(*it);
    }

    static Friend get(ID friendId);
    static Friend::List getAll();

    static void deleteFromProfile(ID friendId);
    static IDList idList();
    static void initCallbacks();
    static void initCache();

private:
    static void updateAvatar(ID friendId, const QPixmap& avatar);

public:
    Friend(Private* p = nullptr);
    Friend(const Friend& other);
    Friend(Friend&& other);
    ~Friend();

    Friend& operator=(const Friend& other);
    Friend& operator=(Friend&& other);

    void loadHistory();
    void invalidate();

    QString getDisplayedName() const;
    bool hasAlias() const;

    QPixmap getAvatar() const;
    QString getStatusMessage();
    ToxId getToxId() const;
    ID getFriendId() const;
    Status getStatus() const;
    bool getTyping() const;

    const OfflineMsgEngine& getOfflineMsgEngine() const;
    void registerReceipt(int rec, qint64 id, ChatMessage::Ptr msg);
    void dischargeReceipt(int receipt);

    void clearOfflineReceipts();
    void deliverOfflineMsgs();

    void setAlias(QString name);

    inline operator bool() const
    {
        return data;
    }

private:
    static FriendNotify notifier;
    static FriendCache friendList;
    static QHash<QString, ID> tox2id;

    PrivatePtr data;
};

class FriendNotify : public QObject
{
    Q_OBJECT

    friend class Friend;

public:
    FriendNotify();

signals:
    void added(Friend f);
    void removed(Friend::ID friendId);
    void failedToRemove(Friend::ID friendId);
    void nameChanged(Friend f, const QString& name);
    void aliasChanged(Friend f, const QString& alias);
    void avatarChanged(Friend f, const QPixmap& getAvatar);
    void statusChanged(Friend f, Status status);
    void statusMessageChanged(Friend f, const QString& message);
    void typingChanged(Friend f, bool isTyping);
    void lastOnlineChanged(Friend f, QDateTime lastOnline);

    // TODO: remove loadHistory signal
    void loadHistory(const Friend& f);
};

#endif // FRIEND_H
