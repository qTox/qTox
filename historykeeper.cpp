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

#include <QSqlError>
#include <QFile>
#include <QDir>
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>

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
        }

        path = "/tmp/qtox_history.sqlite"; // just for testing purpose
        encrypted = false;

        historyInstance = new HistoryKeeper(path, encrypted);
    }

    return historyInstance;
}

HistoryKeeper::HistoryKeeper(const QString &path, bool encr)
{
    Q_UNUSED(encr)

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    if (!db.open())
    {
        qWarning() << QString("Can't open file: %1, history will not be saved!").arg(path);
        db.setDatabaseName(":memory:");
        db.open();
    }

    db.exec(QString("CREATE TABLE IF NOT EXISTS history (id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp DEFAULT CURRENT_TIMESTAMP NOT NULL, ") +
            QString("chat_id INTEGER NOT NULL, sender INTERGER NOT NULL, message TEXT NOT NULL);"));
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

    db.exec(QString("INSERT INTO history (chat_id, sender, message) VALUES (%1, %2, '%3');").arg(chat_id).arg(sender_id).arg(wrapMessage(message)));
}

QList<HistoryKeeper::HistMessage> HistoryKeeper::getChatHistory(HistoryKeeper::ChatType ct, const QString &chat, const QDateTime &time_from, const QDateTime &time_to)
{
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
        //
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
}
