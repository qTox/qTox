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

#ifndef HISTORYKEEPER_H
#define HISTORYKEEPER_H

#include <QMap>
#include <QList>
#include <QDateTime>
#include <QPixmap>
#include <QMutex>
#include <tox/toxencryptsave.h>

class GenericDdInterface;
namespace Db { enum class syncType; }

class HistoryKeeper
{
public:
    enum ChatType {ctSingle = 0, ctGroup};
    static QMutex historyMutex;

    struct HistMessage
    {
        HistMessage(qint64 id, QString chat, QString sender, QString message, QDateTime timestamp, bool isSent, QString dispName) :
            id(id), chat(chat), sender(sender), message(message), timestamp(timestamp), isSent(isSent), dispName(dispName) {}

        qint64 id;
        QString chat;
        QString sender;
        QString message;
        QDateTime timestamp;
        bool isSent;
        QString dispName;
    };

    virtual ~HistoryKeeper();

    static HistoryKeeper* getInstance();
    static void resetInstance();

    static QString getHistoryPath(QString currentProfile = QString(), int encrypted = -1); // -1 defaults to checking settings, 0 or 1 to specify
    static bool checkPassword(const TOX_PASS_KEY& passkey, int encrypted = -1);
    static bool isFileExist();
    static void renameHistory(QString from, QString to);
    void removeHistory();
    static QList<HistMessage> exportMessagesDeleteFile();

    void removeFriendHistory(const QString& chat);
    qint64 addChatEntry(const QString& chat, const QString& message, const QString& sender, const QDateTime &dt, bool isSent, QString dispName);
    qint64 addGroupChatEntry(const QString& chat, const QString& message, const QString& sender, const QDateTime &dt);
    QList<HistMessage> getChatHistory(ChatType ct, const QString &chat, const QDateTime &time_from, const QDateTime &time_to);
    void markAsSent(int m_id);
    QDate getLatestDate(const QString& chat);

    QList<HistMessage> exportMessages();
    void importMessages(const QList<HistoryKeeper::HistMessage> &lst);

    void setSyncType(Db::syncType sType);

    void saveAvatar(QPixmap& pic, const QString& ownerId);
    QPixmap getSavedAvatar(const QString &ownerId);

    void saveAvatarHash(const QByteArray& hash, const QString& ownerId);
    QByteArray getAvatarHash(const QString& ownerId);

    void removeAvatar(const QString& ownerId);
    bool hasAvatar(const QString& ownerId);

    void importAvatarToDatabase(const QString& ownerId);        // may be deleted after all move to new db structure

private:
    HistoryKeeper(GenericDdInterface *db_);
    HistoryKeeper(HistoryKeeper &hk) = delete;
    HistoryKeeper& operator=(const HistoryKeeper&) = delete;

    void updateChatsID();
    void updateAliases();
    QPair<int, ChatType> getChatID(const QString &id_str, ChatType ct);
    int getAliasID(const QString &id_str);
    QString wrapMessage(const QString &str);
    QString unWrapMessage(const QString &str);
    QList<QString> generateAddChatEntryCmd(const QString& chat, const QString& message, const QString& sender, const QDateTime &dt, bool isSent, QString dispName);

    ChatType convertToChatType(int);
    bool needImport = false;    // must be deleted with "importAvatarToDatabase"
    GenericDdInterface *db;
    QMap<QString, int> aliases;
    QMap<QString, QPair<int, ChatType>> chats;
    qint64 messageID;
};

#endif // HISTORYKEEPER_H
