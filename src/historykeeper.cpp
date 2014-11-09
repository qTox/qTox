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

#include "misc/db/plaindb.h"
#include "misc/db/encrypteddb.h"

static HistoryKeeper *historyInstance = nullptr;

HistoryKeeper *HistoryKeeper::getInstance()
{
    if (historyInstance == nullptr)
    {
        QList<QString> initLst;
        initLst.push_back(QString("CREATE TABLE IF NOT EXISTS history (id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp INTEGER NOT NULL, ") +
                          QString("chat_id INTEGER NOT NULL, sender INTEGER NOT NULL, sent_ok INTEGER NOT NULL DEFAULT 0, message TEXT NOT NULL);"));
        initLst.push_back(QString("CREATE TABLE IF NOT EXISTS aliases (id INTEGER PRIMARY KEY AUTOINCREMENT, user_id TEXT UNIQUE NOT NULL);"));
        initLst.push_back(QString("CREATE TABLE IF NOT EXISTS chats (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT UNIQUE NOT NULL, ctype INTEGER NOT NULL);"));

        QString path(":memory:");
        GenericDdInterface *dbIntf;

        if (Settings::getInstance().getEnableLogging())
        {
            bool encrypted = Settings::getInstance().getEncryptLogs();

            if (encrypted)
            {
                path = getHistoryPath();
                dbIntf = new EncryptedDb(path, initLst);

                historyInstance = new HistoryKeeper(dbIntf);
                return historyInstance;
            } else {
                path = getHistoryPath();
            }
        }

        dbIntf = new PlainDb(path, initLst);
        historyInstance = new HistoryKeeper(dbIntf);
    }

    return historyInstance;
}

bool HistoryKeeper::checkPassword()
{
    if (Settings::getInstance().getEnableLogging())
    {
        if (Settings::getInstance().getEncryptLogs())
        {
            QString dbpath = getHistoryPath();
            return EncryptedDb::check(dbpath);
        } else {
            return true;
        }
    } else {
        return true;
    }
}

HistoryKeeper::HistoryKeeper(GenericDdInterface *db_) :
    db(db_)
{
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
       chat_id      -- current chat ID (resolves from chats table)
       sender       -- sender's ID (resolves from aliases table)
       message
    */

    updateChatsID();
    updateAliases();

    QSqlQuery sqlAnswer = db->exec("select seq from sqlite_sequence where name=\"history\";");
    sqlAnswer.first();
    messageID = sqlAnswer.value(0).toInt();
}

HistoryKeeper::~HistoryKeeper()
{
    delete db;
}

int HistoryKeeper::addChatEntry(const QString& chat, const QString& message, const QString& sender, const QDateTime &dt)
{
    int chat_id = getChatID(chat, ctSingle).first;
    int sender_id = getAliasID(sender);
    bool status = sender != Core::getInstance()->getSelfId().publicKey;

    db->exec(QString("INSERT INTO history (timestamp, chat_id, sender, sent_ok, message)") +
             QString("VALUES (%1, %2, %3, %4, '%5');")
             .arg(dt.toMSecsSinceEpoch()).arg(chat_id).arg(sender_id).arg(status).arg(wrapMessage(message)));

    messageID++;
    return messageID;
}

QList<HistoryKeeper::HistMessage> HistoryKeeper::getChatHistory(HistoryKeeper::ChatType ct, const QString &chat,
                                                                const QDateTime &time_from, const QDateTime &time_to)
{
    QList<HistMessage> res;

    qint64 time64_from = time_from.toMSecsSinceEpoch();
    qint64 time64_to = time_to.toMSecsSinceEpoch();

    int chat_id = getChatID(chat, ct).first;

    QSqlQuery dbAnswer;
    if (ct == ctSingle)
    {
        dbAnswer = db->exec(QString("SELECT timestamp, user_id, message, sent_ok FROM history INNER JOIN aliases ON history.sender = aliases.id ") +
                            QString("AND timestamp BETWEEN %1 AND %2 AND chat_id = %3;")
                            .arg(time64_from).arg(time64_to).arg(chat_id));
    } else {
        // no groupchats yet
    }

    while (dbAnswer.next())
    {
        QString sender = dbAnswer.value(1).toString();
        QString message = unWrapMessage(dbAnswer.value(2).toString());
        qint64 timeInt = dbAnswer.value(0).toLongLong();
        QDateTime time = QDateTime::fromMSecsSinceEpoch(timeInt);
        bool isSent = dbAnswer.value(3).toBool();

        res.push_back({sender,message,time,isSent});
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
    auto dbAnswer = db->exec(QString("SELECT * FROM chats;"));

    chats.clear();
    while (dbAnswer.next())
    {
        QString name = dbAnswer.value(1).toString();
        int id = dbAnswer.value(0).toInt();
        ChatType ctype = convertToChatType(dbAnswer.value(2).toInt());

        chats[name] = {id, ctype};
    }
}

void HistoryKeeper::updateAliases()
{
    auto dbAnswer = db->exec(QString("SELECT * FROM aliases;"));

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

    db->exec(QString("INSERT INTO chats (name, ctype) VALUES ('%1', '%2');").arg(id_str).arg(ct));
    updateChatsID();

    return getChatID(id_str, ct);
}

int HistoryKeeper::getAliasID(const QString &id_str)
{
    auto it = aliases.find(id_str);
    if (it != aliases.end())
        return it.value();

    db->exec(QString("INSERT INTO aliases (user_id) VALUES ('%1');").arg(id_str));
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

int HistoryKeeper::addGroupChatEntry(const QString &chat, const QString &message, const QString &sender, const QDateTime &dt)
{
    Q_UNUSED(chat)
    Q_UNUSED(message)
    Q_UNUSED(sender)
    Q_UNUSED(dt)
    // no groupchats yet

    return -1;
}

HistoryKeeper::ChatType HistoryKeeper::convertToChatType(int ct)
{
    if (ct < 0 || ct > 1)
        return ctSingle;

    return static_cast<ChatType>(ct);
}

QString HistoryKeeper::getHistoryPath()
{
    QDir baseDir(Settings::getInstance().getSettingsDirPath());
    QString currentProfile = Settings::getInstance().getCurrentProfile();

    if (Settings::getInstance().getEncryptLogs())
        return baseDir.filePath(currentProfile + ".qtox_history.encrypted");
    else
        return baseDir.filePath(currentProfile + ".qtox_history");
}

void HistoryKeeper::renameHistory(QString from, QString to)
{
    resetInstance();

    QFile fileEnc(QDir(Settings::getInstance().getSettingsDirPath()).filePath(from + ".qtox_history.encrypted"));
    if (fileEnc.exists())
        fileEnc.rename(QDir(Settings::getInstance().getSettingsDirPath()).filePath(to + ".qtox_history.encrypted"));

    QFile filePlain(QDir(Settings::getInstance().getSettingsDirPath()).filePath(from + ".qtox_history"));
    if (filePlain.exists())
        filePlain.rename(QDir(Settings::getInstance().getSettingsDirPath()).filePath(to + ".qtox_history"));
}

void HistoryKeeper::markAsSent(int m_id)
{
    db->exec(QString("UPDATE history SET sent_ok = 1 WHERE id = %1;").arg(m_id));
}
