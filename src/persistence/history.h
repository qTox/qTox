#ifndef HISTORY_H
#define HISTORY_H

#include <QDateTime>

#include "db/userdb.h"

class HistoryKeeper;

class History
{
public:
    struct HistMessage
    {
        HistMessage(qint64 id, bool isSent, QDateTime timestamp, QString chat,
                    QString dispName, QString sender, QString message)
            : id(id), chat(chat), sender(sender), message(message)
            , dispName(dispName), timestamp(timestamp), isSent(isSent)
        {}

        qint64 id;
        QString chat;
        QString sender;
        QString message;
        QString dispName;
        QDateTime timestamp;
        bool isSent;
    };

public:
    History(UserDb* db);
    void eraseHistory();
    void removeFriendHistory(const QString& friendPk);
    void addNewMessage(const QString& friendPk, const QString& message, const QString& sender,
                        const QDateTime &time, bool isSent, QString dispName,
                       std::function<void(int64_t)> insertIdCallback={});

    QList<HistMessage> getChatHistory(const QString& friendPk, const QDateTime &from, const QDateTime &to);
    void markAsSent(qint64 id);

    void import(const HistoryKeeper &oldHistory);
private:
    UserDb* db;
    QVector<RawDatabase::Query> generateNewMessageQueries(
            const QString& friendPk, const QString& message,
            const QString& sender, const QDateTime &time, bool isSent,
            QString dispName, std::function<void(int64_t)> insertIdCallback = {});

    QHash<QString, int64_t> peers;
};

#endif // HISTORY_H
