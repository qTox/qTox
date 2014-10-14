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
#include <QSqlDatabase>
#include <QDateTime>

class HistoryKeeper
{
public:
    enum ChatType {ctSingle = 0, ctGroup};
    enum MessageType {mtMessage = 0, mtAction};

    struct HistMessage
    {
        QString sender;
        QString message;
        QDateTime timestamp;
        MessageType mt;
    };

    static HistoryKeeper* getInstance();
    static void resetInstance();
    virtual ~HistoryKeeper();

    void addChatEntry(const QString& chat, MessageType mt, const QString& message, const QString& sender, const QDateTime &dt);
    void addGroupChatEntry(const QString& chat, const QString& message, const QString& sender, const QDateTime &dt);
    QList<HistMessage> getChatHistory(ChatType ct, const QString &profile, const QString &chat,
                                      const QDateTime &time_from, const QDateTime &time_to);
    void syncToDisk();

private:

    HistoryKeeper(const QString &path, bool encr = false);
    HistoryKeeper(HistoryKeeper &settings) = delete;
    HistoryKeeper& operator=(const HistoryKeeper&) = delete;

    void updateChatsID();
    void updateAliases();
    QPair<int, ChatType> getChatID(const QString &id_str, ChatType ct);
    int getAliasID(const QString &id_str);
    int getCurrentProfileID();
    QString wrapMessage(const QString &str);
    QString unWrapMessage(const QString &str);
    bool dumpDBtoFile(const QString &fname);

    MessageType convertToMessageType(int);
    ChatType convertToChatType(int);

    QSqlDatabase db;
    QMap<QString, int> aliases;
    QMap<QString, QPair<int, ChatType>> chats;
    bool isEncrypted;
};

#endif // HISTORYKEEPER_H
