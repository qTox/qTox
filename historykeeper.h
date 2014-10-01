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
    struct HistMessage
    {
        QString sender;
        QString message;
        QDateTime timestamp;
    };
    enum ChatType {ctSingle = 0, ctGroup};

    static HistoryKeeper* getInstance();
    static void resetInstance();
    virtual ~HistoryKeeper();

    void addChatEntry(const QString& chat, const QString& message, const QString& sender);
    void addGroupChatEntry(const QString& chat, const QString& message, const QString& sender);
    QList<HistMessage> getChatHistory(ChatType ct, const QString &chat, const QDateTime &time_from, const QDateTime &time_to = QDateTime::currentDateTimeUtc());

private:

    HistoryKeeper(const QString &path, bool encr = false);
    HistoryKeeper(HistoryKeeper &settings) = delete;
    HistoryKeeper& operator=(const HistoryKeeper&) = delete;

    void updateChatsID();
    void updateAliases();
    QPair<int, ChatType> getChatID(const QString &id_str, ChatType ct);
    int getAliasID(const QString &id_str);
    QString wrapMessage(const QString &str);
    QString unWrapMessage(const QString &str);

    QSqlDatabase db;
    QMap<QString, int> aliases;
    QMap<QString, QPair<int, ChatType>> chats;
};

#endif // HISTORYKEEPER_H
