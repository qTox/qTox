/*
    Copyright © 2015-2019 by The qTox Project Contributors

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

#pragma once

#include <QDateTime>
#include <QHash>
#include <QPointer>
#include <QVector>

#include <cassert>
#include <cstdint>
#include <tox/toxencryptsave.h>

#include "src/core/extension.h"
#include "src/core/toxfile.h"
#include "src/core/toxpk.h"
#include "src/model/brokenmessagereason.h"
#include "src/model/systemmessage.h"
#include "src/persistence/db/rawdatabase.h"
#include "src/widget/searchtypes.h"

class Profile;
class HistoryKeeper;
class Settings;
class ChatId;

enum class HistMessageContentType
{
    message,
    file,
    system,
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

    HistMessageContent(SystemMessage systemMessage)
        : data(std::make_shared<SystemMessage>(std::move(systemMessage)))
        , type(HistMessageContentType::system)
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

    SystemMessage& asSystemMessage()
    {
        assert(type == HistMessageContentType::system);
        return *static_cast<SystemMessage*>(data.get());
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

    const SystemMessage& asSystemMessage() const
    {
        assert(type == HistMessageContentType::system);
        return *static_cast<SystemMessage*>(data.get());
    }

private:
    // Not really shared but shared_ptr has support for shared_ptr<void>
    std::shared_ptr<void> data;
    HistMessageContentType type;
};

struct FileDbInsertionData
{
    FileDbInsertionData();

    RowId historyId;
    QByteArray fileId;
    QString fileName;
    QString filePath;
    int64_t size;
    int direction;
};
Q_DECLARE_METATYPE(FileDbInsertionData)

enum class MessageState
{
    complete,
    pending,
    broken
};

class History : public QObject, public std::enable_shared_from_this<History>
{
    Q_OBJECT
public:
    struct HistMessage
    {
        HistMessage(RowId id_, MessageState state_, ExtensionSet extensionSet_, QDateTime timestamp_, std::unique_ptr<ChatId> chat_, QString dispName_,
                    ToxPk sender_, QString message)
            : chat{std::move(chat_)}
            , sender{sender_}
            , dispName{dispName_}
            , timestamp{timestamp_}
            , id{id_}
            , state{state_}
            , extensionSet(extensionSet_)
            , content(std::move(message))
        {}

        HistMessage(RowId id_, MessageState state_, QDateTime timestamp_, std::unique_ptr<ChatId> chat_, QString dispName_,
                    ToxPk sender_, ToxFile file)
            : chat{std::move(chat_)}
            , sender{sender_}
            , dispName{dispName_}
            , timestamp{timestamp_}
            , id{id_}
            , state{state_}
            , content(std::move(file))
        {}

        HistMessage(RowId id_, QDateTime timestamp_, std::unique_ptr<ChatId> chat_, SystemMessage systemMessage)
            : chat{std::move(chat_)}
            , timestamp{timestamp_}
            , id{id_}
            , state(MessageState::complete)
            , content(std::move(systemMessage))
        {}

        HistMessage(const History::HistMessage& other)
            : chat{other.chat->clone()}
            , sender{other.sender}
            , dispName{other.dispName}
            , timestamp{other.timestamp}
            , id{other.id}
            , state{other.state}
            , extensionSet{other.extensionSet}
            , content{other.content}
        {}

        HistMessage& operator=(const HistMessage& other)
        {
            chat = other.chat->clone();
            sender = other.sender;
            dispName = other.dispName;
            timestamp = other.timestamp;
            id = other.id;
            state = other.state;
            extensionSet = other.extensionSet;
            content = other.content;
            return *this;
        }

        std::unique_ptr<ChatId> chat;
        ToxPk sender;
        QString dispName;
        QDateTime timestamp;
        RowId id;
        MessageState state;
        ExtensionSet extensionSet;
        HistMessageContent content;
    };

    struct DateIdx
    {
        QDate date;
        size_t numMessagesIn;
    };

public:
    History(std::shared_ptr<RawDatabase> db, Settings& settings);
    ~History();

    bool isValid();

    bool historyExists(const ChatId& chatId);

    void eraseHistory();
    void removeChatHistory(const ChatId& chatId);
    void addNewMessage(const ChatId& chatId, const QString& message, const ToxPk& sender,
                       const QDateTime& time, bool isDelivered, ExtensionSet extensions,
                       QString dispName, const std::function<void(RowId)>& insertIdCallback = {});

    void addNewFileMessage(const ChatId& chatId, const QByteArray& fileId,
                           const QString& fileName, const QString& filePath, int64_t size,
                           const ToxPk& sender, const QDateTime& time, QString const& dispName);

    void addNewSystemMessage(const ChatId& chatId, const SystemMessage& systemMessage);

    void setFileFinished(const QByteArray& fileId, bool success, const QString& filePath, const QByteArray& fileHash);
    size_t getNumMessagesForChat(const ChatId& chatId);
    size_t getNumMessagesForChatBeforeDate(const ChatId& chatId, const QDateTime& date);
    QList<HistMessage> getMessagesForChat(const ChatId& chatId, size_t firstIdx, size_t lastIdx);
    QList<HistMessage> getUndeliveredMessagesForChat(const ChatId& chatId);
    QDateTime getDateWhereFindPhrase(const ChatId& chatId, const QDateTime& from, QString phrase,
                                     const ParameterSearch& parameter);
    QList<DateIdx> getNumMessagesForChatBeforeDateBoundaries(const ChatId& chatId,
                                                               const QDate& from, size_t maxNum);

    void markAsDelivered(RowId messageId);
    void markAsBroken(RowId messageId, BrokenMessageReason reason);

signals:
    void fileInserted(RowId dbId, QByteArray fileId);

private slots:
    void onFileInserted(RowId dbId, QByteArray fileId);

private:
    QVector<RawDatabase::Query>
    generateNewFileTransferQueries(const ChatId& chatId, const ToxPk& sender, const QDateTime& time,
                                   const QString& dispName, const FileDbInsertionData& insertionData);
    bool historyAccessBlocked();
    static RawDatabase::Query generateFileFinished(RowId fileId, bool success,
                                                   const QString& filePath, const QByteArray& fileHash);

    std::shared_ptr<RawDatabase> db;


    struct FileInfo
    {
        bool finished = false;
        bool success = false;
        QString filePath;
        QByteArray fileHash;
        RowId fileId{-1};
    };

    // This needs to be a shared pointer to avoid callback lifetime issues
    QHash<QByteArray, FileInfo> fileInfos;
    Settings& settings;
};
