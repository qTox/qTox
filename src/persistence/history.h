/*
    Copyright © 2015-2018 by The qTox Project Contributors

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
#include <QHash>
#include <QPointer>
#include <QVector>

#include <cassert>
#include <cstdint>
#include <tox/toxencryptsave.h>

#include "src/core/toxfile.h"
#include "src/core/toxpk.h"
#include "src/persistence/db/rawdatabase.h"
#include "src/widget/searchtypes.h"

class Profile;
class HistoryKeeper;

enum class HistMessageContentType
{
    message,
    file
};

class HistMessageContent
{
public:
    HistMessageContent(QString message)
        : data(std::make_shared<QString>(std::move(message)))
        , type(HistMessageContentType::message)
    {}

    HistMessageContent(ToxFile file)
        : data(std::make_shared<ToxFile>(std::move(file)))
        , type(HistMessageContentType::file)
    {}

    HistMessageContentType getType() const
    {
        return type;
    }

    QString& asMessage()
    {
        assert(type == HistMessageContentType::message);
        return *static_cast<QString*>(data.get());
    }

    ToxFile& asFile()
    {
        assert(type == HistMessageContentType::file);
        return *static_cast<ToxFile*>(data.get());
    }

    const QString& asMessage() const
    {
        assert(type == HistMessageContentType::message);
        return *static_cast<QString*>(data.get());
    }

    const ToxFile& asFile() const
    {
        assert(type == HistMessageContentType::file);
        return *static_cast<ToxFile*>(data.get());
    }

private:
    // Not really shared but shared_ptr has support for shared_ptr<void>
    std::shared_ptr<void> data;
    HistMessageContentType type;
};

class History
{
public:
    struct HistMessage
    {
        HistMessage(qint64 id, bool isSent, QDateTime timestamp, QString chat, QString dispName,
                    QString sender, QString message)
            : chat{chat}
            , sender{sender}
            , dispName{dispName}
            , timestamp{timestamp}
            , id{id}
            , isSent{isSent}
            , content(std::move(message))
        {}

        HistMessage(qint64 id, bool isSent, QDateTime timestamp, QString chat, QString dispName,
                    QString sender, ToxFile file)
            : chat{chat}
            , sender{sender}
            , dispName{dispName}
            , timestamp{timestamp}
            , id{id}
            , isSent{isSent}
            , content(std::move(file))
        {}


        QString chat;
        QString sender;
        QString dispName;
        QDateTime timestamp;
        qint64 id;
        bool isSent;
        HistMessageContent content;
    };

    struct DateMessages
    {
        uint offsetDays;
        uint count;
    };

public:
    explicit History(std::shared_ptr<RawDatabase> db);
    ~History();

    bool isValid();
    void import(const HistoryKeeper& oldHistory);

    void eraseHistory();
    void removeFriendHistory(const QString& friendPk);
    void addNewMessage(const QString& friendPk, const QString& message, const QString& sender,
                       const QDateTime& time, bool isSent, QString dispName,
                       const std::function<void(int64_t)>& insertIdCallback = {});

    void addNewFileMessage(const QString& friendPk, const QString& fileId,
                           const QByteArray& fileName, const QString& filePath, int64_t size,
                           const QString& sender, const QDateTime& time, QString const& dispName);

    void setFileFinished(const QString& fileId, bool success, const QString& filePath);

    QList<HistMessage> getChatHistoryFromDate(const QString& friendPk, const QDateTime& from,
                                              const QDateTime& to);
    QList<HistMessage> getChatHistoryDefaultNum(const QString& friendPk);
    QList<DateMessages> getChatHistoryCounts(const ToxPk& friendPk, const QDate& from, const QDate& to);
    QDateTime getDateWhereFindPhrase(const QString& friendPk, const QDateTime& from, QString phrase,
                                     const ParameterSearch& parameter);
    QDateTime getStartDateChatHistory(const QString& friendPk);

    void markAsSent(qint64 messageId);

protected:
    QVector<RawDatabase::Query>
    generateNewMessageQueries(const QString& friendPk, const QString& message,
                              const QString& sender, const QDateTime& time, bool isSent,
                              QString dispName, std::function<void(int64_t)> insertIdCallback = {});


private:
    QList<HistMessage> getChatHistory(const QString& friendPk, const QDateTime& from,
                                      const QDateTime& to, int numMessages);

    static RawDatabase::Query generateFileFinished(int64_t fileId, bool success,
                                                   const QString& filePath);
    void dbSchemaUpgrade();

    std::shared_ptr<RawDatabase> db;
    using Peers = QHash<QString, int64_t>;
    std::shared_ptr<Peers> peers;
    struct FileInfo
    {
        bool finished = false;
        bool success = false;
        QString filePath;
        int64_t fileId = -1;
    };

    // This needs to be a shared pointer to avoid callback lifetime issues
    using FileInfos = QHash<QString, FileInfo>;
    std::shared_ptr<FileInfos> fileInfos;
};

#endif // HISTORY_H
