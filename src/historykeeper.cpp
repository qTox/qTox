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

#include "historykeeper.h"
#include "misc/settings.h"
#include "core.h"

#include <QSqlError>
#include <QFile>
#include <QDir>
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>
#include <QTemporaryFile>

static HistoryKeeper *historyInstance = nullptr;

HistoryKeeper *HistoryKeeper::getInstance()
{
    if (historyInstance == nullptr)
    {
        QString path(":memory:");
        bool encrypted = Settings::getInstance().getEncryptLogs();

        if (Settings::getInstance().getEnableLogging())
        {
            path = QDir(Settings::getInstance().getSettingsDirPath()).filePath("qtox_history.sqlite");
            // path = "/tmp/qtox_history.sqlite"; // just for testing purpose
        }

        historyInstance = new HistoryKeeper(path, encrypted);
    }

    return historyInstance;
}

HistoryKeeper::HistoryKeeper(const QString &path, bool encr) :
    isEncrypted(encr)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    if (isEncrypted)
        db.setDatabaseName(":memory:");
    else
        db.setDatabaseName(path);

    if (!db.open())
    {
        qWarning() << QString("Can't open file: %1, history will not be saved!").arg(path);
        db.setDatabaseName(":memory:");
        db.open();
    }

    /*
     DB format
     chats:
      * name -> id map
       id      -- auto-incrementing number
       name    -- chat's name (for user to user conversation it is opposite user public key)
       ctype   -- chat type, reserved for group chats

     alisases:
      * user_id -> id map
       id      -- auto-incrementing number
       name    -- user's public key

     history:
       id           -- auto-incrementing number
       timestamp
       profile_id   -- profile ID (resolves from aliases table)
       chat_id      -- current chat ID (resolves from chats table)
       sender       -- sender's ID (resolves from aliases table)
       mtype        -- type of message (message, action, etc) (UNUSED now)
       message
    */

    db.exec(QString("CREATE TABLE IF NOT EXISTS history (id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp DEFAULT CURRENT_TIMESTAMP NOT NULL, ") +
            QString("profile_id INTEGER NOT NULL, chat_id INTEGER NOT NULL, sender INTERGER NOT NULL, mtype INTEGER NOT NULL, message TEXT NOT NULL);"));
    db.exec(QString("CREATE TABLE IF NOT EXISTS aliases (id INTEGER PRIMARY KEY AUTOINCREMENT, user_id TEXT UNIQUE NOT NULL);"));
    db.exec(QString("CREATE TABLE IF NOT EXISTS chats (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT UNIQUE NOT NULL, ctype INTEGER NOT NULL);"));

    updateChatsID();
    updateAliases();
}

HistoryKeeper::~HistoryKeeper()
{
    db.close();
}

void HistoryKeeper::addChatEntry(const QString& chat, const QString& message, const QString& sender)
{
    int chat_id = getChatID(chat, ctSingle).first;
    int sender_id = getAliasID(sender);
    int profile_id = getCurrentProfileID();

    db.exec(QString("INSERT INTO history (profile_id, chat_id, sender, mtype, message) VALUES (%1, %2, %3, 0, '%4');")
            .arg(profile_id).arg(chat_id).arg(sender_id).arg(wrapMessage(message)));
}

QList<HistoryKeeper::HistMessage> HistoryKeeper::getChatHistory(HistoryKeeper::ChatType ct, const QString &profile,
                                                                const QString &chat, const QDateTime &time_from,
                                                                const QDateTime &time_to)
{
    Q_UNUSED(profile)

    QList<HistMessage> res;

    QString timeStr_from = time_from.toString("yyyy-MM-dd hh:mm:ss");
    QString timeStr_to = time_to.toString("yyyy-MM-dd hh:mm:ss");

    int chat_id = getChatID(chat, ct).first;

    QSqlQuery dbAnswer;
    if (ct == ctSingle)
    {
        dbAnswer = db.exec(QString("SELECT timestamp, user_id, message FROM history INNER JOIN aliases ON history.sender = aliases.id ") +
                           QString("AND timestamp BETWEEN '%1' AND '%2' AND chat_id = %3;").arg(timeStr_from).arg(timeStr_to).arg(chat_id));
    } else {
        // no groupchats yet
    }

    while (dbAnswer.next())
    {
        QString sender = dbAnswer.value(1).toString();
        QString message = unWrapMessage(dbAnswer.value(2).toString());
        QDateTime time = dbAnswer.value(0).toDateTime();
        time.setTimeSpec(Qt::UTC);

        res.push_back({sender,message,time});
    }

    return res;
}

QString HistoryKeeper::wrapMessage(const QString &str)
{
    QString wrappedMessage(str);
    wrappedMessage.replace("'", "''");
    return wrappedMessage;
}

QString HistoryKeeper::unWrapMessage(const QString &str)
{
    QString unWrappedMessage(str);
    unWrappedMessage.replace("''", "'");
    return unWrappedMessage;
}

void HistoryKeeper::updateChatsID()
{
    auto dbAnswer = db.exec(QString("SELECT * FROM chats;"));

    chats.clear();
    while (dbAnswer.next())
    {
        QString name = dbAnswer.value(1).toString();
        int id = dbAnswer.value(0).toInt();
        ChatType ctype = static_cast<ChatType>(dbAnswer.value(2).toInt());

        chats[name] = {id, ctype};
    }
}

void HistoryKeeper::updateAliases()
{
    auto dbAnswer = db.exec(QString("SELECT * FROM aliases;"));

    aliases.clear();
    while (dbAnswer.next())
    {
        QString user_id = dbAnswer.value(1).toString();
        int id = dbAnswer.value(0).toInt();

        aliases[user_id] = id;
    }
}

QPair<int, HistoryKeeper::ChatType> HistoryKeeper::getChatID(const QString &id_str, ChatType ct)
{
    auto it = chats.find(id_str);
    if (it != chats.end())
        return it.value();

    db.exec(QString("INSERT INTO chats (name, ctype) VALUES ('%1', '%2');").arg(id_str).arg(ct));
    updateChatsID();

    return getChatID(id_str, ct);
}

int HistoryKeeper::getAliasID(const QString &id_str)
{
    auto it = aliases.find(id_str);
    if (it != aliases.end())
        return it.value();

    db.exec(QString("INSERT INTO aliases (user_id) VALUES ('%1');").arg(id_str));
    updateAliases();

    return getAliasID(id_str);
}

void HistoryKeeper::resetInstance()
{
    if (historyInstance == nullptr)
        return;

    delete historyInstance;
    historyInstance = nullptr;
}

void HistoryKeeper::addGroupChatEntry(const QString &chat, const QString &message, const QString &sender)
{
    Q_UNUSED(chat)
    Q_UNUSED(message)
    Q_UNUSED(sender)
    // no groupchats yet
}

void HistoryKeeper::syncToDisk()
{
    if (!isEncrypted)
        return;

    QTemporaryFile tmpDump;
    if (tmpDump.open())
    {
        QString fname = tmpDump.fileName();
        if (!dumpDBtoFile(fname))
        {
            // warning
            return;
        }

        // encryption here
    } else
    {
        // warning
    }
}

bool HistoryKeeper::dumpDBtoFile(const QString &fname)
{
    Q_UNUSED(fname)
    return false;
}

int HistoryKeeper::getCurrentProfileID()
{
    // for many profiles
    return getAliasID(Core::getInstance()->getSelfId().publicKey);
}
