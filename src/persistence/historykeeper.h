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

/**
 * THIS IS A LEGACY CLASS KEPT FOR BACKWARDS COMPATIBILITY
 * DO NOT USE!
 * See the History class instead
 */

class Profile;
class GenericDdInterface;
namespace Db { enum class syncType; }

class HistoryKeeper
{
public:
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

    static HistoryKeeper* getInstance(const Profile& profile);
    static void resetInstance();

    static QString getHistoryPath(QString currentProfile = QString(), int encrypted = -1); // -1 defaults to checking settings, 0 or 1 to specify
    static bool checkPassword(const TOX_PASS_KEY& passkey, int encrypted = -1);
    static bool isFileExist(bool encrypted);
    void removeHistory();
    static QList<HistMessage> exportMessagesDeleteFile();
    QList<HistMessage> exportMessages();

private:
    HistoryKeeper(GenericDdInterface *db_);
    HistoryKeeper(HistoryKeeper &hk) = delete;
    HistoryKeeper& operator=(const HistoryKeeper&) = delete;
    QString unWrapMessage(const QString &str);

    GenericDdInterface *oldDb;
};

#endif // HISTORYKEEPER_H
