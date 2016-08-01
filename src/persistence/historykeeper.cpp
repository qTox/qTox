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

#include "historykeeper.h"
#include "src/persistence/settings.h"
#include "src/core/core.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"

#include <QSqlError>
#include <QFile>
#include <QDir>
#include <QSqlQuery>
#include <QVariant>
#include <QBuffer>
#include <QDebug>
#include <QTemporaryFile>

#include "src/persistence/db/plaindb.h"
#include "src/persistence/db/encrypteddb.h"

/**
 * @class HistoryKeeper
 * @brief THIS IS A LEGACY CLASS KEPT FOR BACKWARDS COMPATIBILITY
 * @deprecated See the History class instead
 * @warning DO NOT USE!
 */

static HistoryKeeper *historyInstance = nullptr;
QMutex HistoryKeeper::historyMutex;

/**
 * @brief Returns the singleton instance.
 */
HistoryKeeper *HistoryKeeper::getInstance(const Profile& profile)
{
    historyMutex.lock();
    if (historyInstance == nullptr)
    {
        QList<QString> initLst;
        initLst.push_back(QString("CREATE TABLE IF NOT EXISTS history (id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp INTEGER NOT NULL, ") +
                          QString("chat_id INTEGER NOT NULL, sender INTEGER NOT NULL, message TEXT NOT NULL, alias TEXT);"));
        initLst.push_back(QString("CREATE TABLE IF NOT EXISTS aliases (id INTEGER PRIMARY KEY AUTOINCREMENT, user_id TEXT UNIQUE NOT NULL);"));
        initLst.push_back(QString("CREATE TABLE IF NOT EXISTS chats (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT UNIQUE NOT NULL, ctype INTEGER NOT NULL);"));
        initLst.push_back(QString("CREATE TABLE IF NOT EXISTS sent_status (id INTEGER PRIMARY KEY AUTOINCREMENT, status INTEGER NOT NULL DEFAULT 0);"));

        QString path(":memory:");
        GenericDdInterface *dbIntf;

        if (profile.isEncrypted())
        {
            path = getHistoryPath({}, 1);
            dbIntf = new EncryptedDb(path, initLst);

            historyInstance = new HistoryKeeper(dbIntf);
            historyMutex.unlock();
            return historyInstance;
        }
        else
        {
            path = getHistoryPath({}, 0);
        }

        dbIntf = new PlainDb(path, initLst);
        historyInstance = new HistoryKeeper(dbIntf);
    }
    historyMutex.unlock();

    return historyInstance;
}

bool HistoryKeeper::checkPassword(const TOX_PASS_KEY &passkey, int encrypted)
{
    if (!Settings::getInstance().getEnableLogging() && (encrypted == -1))
        return true;

    if ((encrypted == 1) || (encrypted == -1))
        return EncryptedDb::check(passkey, getHistoryPath(Nexus::getProfile()->getName(), encrypted));

    return true;
}

HistoryKeeper::HistoryKeeper(GenericDdInterface *db_) :
    oldDb(db_)
{
    /*
     DB format
     chats:
      * name -> id map
       id      -- auto-incrementing number
       name    -- chat's name (for user to user conversation it is opposite user public key)
       ctype   -- chat type, reserved for group chats

     aliases:
      * user_id -> id map
       id       -- auto-incrementing number
       user_id  -- user's public key
       av_hash  -- hash of user's avatar
       avatar   -- user's avatar

     history:
       id           -- auto-incrementing number
       timestamp
       chat_id      -- current chat ID (resolves from chats table)
       sender       -- sender's ID (resolves from aliases table)
       message
       alias        -- sender's alias in
    */

    // for old tables:
    QSqlQuery ans = oldDb->exec("SELECT seq FROM sqlite_sequence WHERE name=\"history\";");
    if (ans.first())
    {
        int idMax = ans.value(0).toInt();
        QSqlQuery ret = oldDb->exec("SELECT seq FROM sqlite_sequence WHERE name=\"sent_status\";");
        int idCur = 0;
        if (ret.first())
            idCur = ret.value(0).toInt();

        if (idCur != idMax)
        {
            QString cmd = QString("INSERT INTO sent_status (id, status) VALUES (%1, 1);").arg(idMax);
            oldDb->exec(cmd);
        }
    }
    //check table stuct
    ans = oldDb->exec("PRAGMA table_info (\"history\")");
    ans.seek(5);
    if (!ans.value(1).toString().contains("alias"))
    {
        //add collum in table
        oldDb->exec("ALTER TABLE history ADD COLUMN alias TEXT");
        qDebug() << "Struct DB updated: Added column alias in table history.";
    }
}

HistoryKeeper::~HistoryKeeper()
{
    delete oldDb;
}

QList<HistoryKeeper::HistMessage> HistoryKeeper::exportMessages()
{
    QSqlQuery dbAnswer;
    dbAnswer = oldDb->exec(QString("SELECT history.id, timestamp, user_id, message, status, name, alias FROM history LEFT JOIN sent_status ON history.id = sent_status.id ") +
                        QString("INNER JOIN aliases ON history.sender = aliases.id INNER JOIN chats ON history.chat_id = chats.id;"));

    QList<HistMessage> res;

    while (dbAnswer.next())
    {
        qint64 id = dbAnswer.value(0).toLongLong();
        qint64 timeInt = dbAnswer.value(1).toLongLong();
        QString sender = dbAnswer.value(2).toString();
        QString message = unWrapMessage(dbAnswer.value(3).toString());
        bool isSent = true;
        if (!dbAnswer.value(4).isNull())
            isSent = dbAnswer.value(4).toBool();
        QString chat = dbAnswer.value(5).toString();
        QString dispName = dbAnswer.value(6).toString();
        QDateTime time = QDateTime::fromMSecsSinceEpoch(timeInt);

        res.push_back(HistMessage(id, chat, sender, message, time, isSent, dispName));
    }

    return res;
}

QString HistoryKeeper::unWrapMessage(const QString &str)
{
    QString unWrappedMessage(str);
    unWrappedMessage.replace("''", "'");
    return unWrappedMessage;
}

void HistoryKeeper::resetInstance()
{
    if (historyInstance == nullptr)
        return;

    delete historyInstance;
    historyInstance = nullptr;
}

QString HistoryKeeper::getHistoryPath(QString currentProfile, int encrypted)
{
    QDir baseDir(Settings::getInstance().getSettingsDirPath());
    if (currentProfile.isEmpty())
        currentProfile = Settings::getInstance().getCurrentProfile();

    if (encrypted == 1 || (encrypted == -1 && Nexus::getProfile()->isEncrypted()))
        return baseDir.filePath(currentProfile + ".qtox_history.encrypted");
    else
        return baseDir.filePath(currentProfile + ".qtox_history");
}

bool HistoryKeeper::isFileExist(bool encrypted)
{
    QString path = getHistoryPath({}, encrypted ? 1 : 0);
    QFile file(path);

    return file.exists();
}

void HistoryKeeper::removeHistory()
{
    resetInstance();
    QFile::remove(getHistoryPath({}, 0));
    QFile::remove(getHistoryPath({}, 1));
}

QList<HistoryKeeper::HistMessage> HistoryKeeper::exportMessagesDeleteFile()
{
    auto msgs = getInstance(*Nexus::getProfile())->exportMessages();
    qDebug() << "Messages exported";
    getInstance(*Nexus::getProfile())->removeHistory();

    return msgs;
}
