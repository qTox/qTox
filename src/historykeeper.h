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

#ifndef HISTORYKEEPER_H
#define HISTORYKEEPER_H

#include <QMap>
#include <QList>
#include <QDateTime>

class GenericDdInterface;
namespace Db { enum class syncType; }

class HistoryKeeper
{
public:
    enum ChatType {ctSingle = 0, ctGroup};

    struct HistMessage
    {
        HistMessage(qint64 id, QString chat, QString sender, QString message, QDateTime timestamp, bool isSent) :
            id(id), chat(chat), sender(sender), message(message), timestamp(timestamp), isSent(isSent) {}

        qint64 id;
        QString chat;
        QString sender;
        QString message;
        QDateTime timestamp;
        bool isSent;
    };

    virtual ~HistoryKeeper();

    static HistoryKeeper* getInstance();
    static void resetInstance();

    static QString getHistoryPath(QString currentProfile = QString(), int encrypted = -1); // -1 defaults to checking settings, 0 or 1 to specify
    static bool checkPassword(int encrypted = -1);
    static bool isFileExist();
    static void renameHistory(QString from, QString to);
    static bool removeHistory(int encrypted = -1);
    static QList<HistMessage> exportMessagesDeleteFile(int encrypted = -1);

    void removeFriendHistory(const QString& chat);
    qint64 addChatEntry(const QString& chat, const QString& message, const QString& sender, const QDateTime &dt, bool isSent);
    qint64 addGroupChatEntry(const QString& chat, const QString& message, const QString& sender, const QDateTime &dt);
    QList<HistMessage> getChatHistory(ChatType ct, const QString &chat, const QDateTime &time_from, const QDateTime &time_to);
    void markAsSent(int m_id);

    QList<HistMessage> exportMessages();
    void importMessages(const QList<HistoryKeeper::HistMessage> &lst);

    void setSyncType(Db::syncType sType);

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
    QList<QString> generateAddChatEntryCmd(const QString& chat, const QString& message, const QString& sender, const QDateTime &dt, bool isSent);

    ChatType convertToChatType(int);

    GenericDdInterface *db;
    QMap<QString, int> aliases;
    QMap<QString, QPair<int, ChatType>> chats;
    qint64 messageID;
};

#endif // HISTORYKEEPER_H
