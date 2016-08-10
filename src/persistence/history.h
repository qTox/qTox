#ifndef HISTORY_H
#define HISTORY_H

#include <tox/toxencryptsave.h>
#include <QDateTime>
#include <QVector>
#include <QHash>
#include <cstdint>
#include "src/persistence/db/rawdatabase.h"

class Profile;
class HistoryKeeper;
class RawDatabase;

class History
{   
public:
    struct HistMessage
    {
        HistMessage(qint64 id, bool isSent, QDateTime timestamp, QString chat, QString dispName, QString sender, QString message) :
            chat{chat}, sender{sender}, message{message}, dispName{dispName}, timestamp{timestamp}, id{id}, isSent{isSent} {}

        QString chat;
        QString sender;
        QString message;
        QString dispName;
        QDateTime timestamp;
        qint64 id;
        bool isSent;
    };

public:
    History(const QString& profileName, const QString& password);
    History(const QString& profileName, const QString& password, const HistoryKeeper& oldHistory);
    ~History();

    bool isValid();
    void import(const HistoryKeeper& oldHistory);
    void setPassword(const QString& password);
    void rename(const QString& newName);
    bool remove();

    void eraseHistory();
    void removeFriendHistory(const QString& friendPk);
    void addNewMessage(const QString& friendPk, const QString& message, const QString& sender,
                        const QDateTime &time, bool isSent, QString dispName,
                       std::function<void(int64_t)> insertIdCallback={});

    QList<HistMessage> getChatHistory(const QString& friendPk, const QDateTime &from, const QDateTime &to);
    void markAsSent(qint64 id);
    static QString getDbPath(const QString& profileName);
protected:
    void init();
    QVector<RawDatabase::Query> generateNewMessageQueries(const QString& friendPk, const QString& message,
                                    const QString& sender, const QDateTime &time, bool isSent, QString dispName,
                                                          std::function<void(int64_t)> insertIdCallback={});

private:
    RawDatabase db;
    QHash<QString, int64_t> peers;
};

#endif // HISTORY_H
