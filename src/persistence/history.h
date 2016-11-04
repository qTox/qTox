/*
    Copyright © 2015-2016 by The qTox Project

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

#ifndef HISTORY_H
#define HISTORY_H

#include <QDateTime>
#include <QVector>
#include <QHash>

#include <cstdint>
#include <tox/toxencryptsave.h>

#include "src/persistence/db/rawdatabase.h"

class Profile;
class HistoryKeeper;

class History
{   
public:
    struct HistMessage
    {
        HistMessage(qint64 id, bool isSent, QDateTime timestamp, QString chat,
                    QString dispName, QString sender, QString message)
            : chat{chat}, sender{sender}, message{message}, dispName{dispName}
            , timestamp{timestamp}, id{id}, isSent{isSent}
        {
        }

        QString chat;
        QString sender;
        QString message;
        QString dispName;
        QDateTime timestamp;
        qint64 id;
        bool isSent;
    };

public:
    History(std::shared_ptr<RawDatabase> db);
    ~History();

    bool isValid();
    void import(const HistoryKeeper& oldHistory);

    void eraseHistory();
    void removeFriendHistory(const QString& friendPk);
    void addNewMessage(const QString& friendPk, const QString& message,
                       const QString& sender, const QDateTime& time,
                       bool isSent, QString dispName,
                       std::function<void(int64_t)> insertIdCallback={});

    QList<HistMessage> getChatHistory(const QString& friendPk,
                                      const QDateTime& from,
                                      const QDateTime& to);
    void markAsSent(qint64 messageId);
protected:
    QVector<RawDatabase::Query> generateNewMessageQueries(
            const QString& friendPk, const QString& message,
            const QString& sender, const QDateTime& time,
            bool isSent, QString dispName,
            std::function<void(int64_t)> insertIdCallback={});

private:
    std::shared_ptr<RawDatabase> db;
    QHash<QString, int64_t> peers;
};

#endif // HISTORY_H
