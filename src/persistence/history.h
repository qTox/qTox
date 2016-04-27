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

/// Interacts with the profile database to save the chat history
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
    /// Opens the profile database and prepares to work with the history
    /// If password is empty, the database will be opened unencrypted
    History(const QString& profileName, const QString& password);
    /// Opens the profile database, and import from the old database
    /// If password is empty, the database will be opened unencrypted
    History(const QString& profileName, const QString& password, const HistoryKeeper& oldHistory);
    ~History();
    /// Checks if the database was opened successfully
    bool isValid();
    /// Imports messages from the old history file
    void import(const HistoryKeeper& oldHistory);
    /// Changes the database password, will encrypt or decrypt if necessary
    void setPassword(const QString& password);
    /// Moves the database file on disk to match the new name
    void rename(const QString& newName);
    /// Deletes the on-disk database file
    bool remove();

    /// Erases all the chat history from the database
    void eraseHistory();
    /// Erases the chat history with one friend
    void removeFriendHistory(const QString& friendPk);
    /// Saves a chat message in the database
    void addNewMessage(const QString& friendPk, const QString& message, const QString& sender,
                        const QDateTime &time, bool isSent, QString dispName,
                       std::function<void(int64_t)> insertIdCallback={});
    /// Fetches chat messages from the database
    QList<HistMessage> getChatHistory(const QString& friendPk, const QDateTime &from, const QDateTime &to);
    /// Marks a message as sent, removing it from the faux-offline pending messages list
    void markAsSent(qint64 id);
    /// Retrieves the path to the database file for a given profile.
    static QString getDbPath(const QString& profileName);
protected:
    /// Makes sure the history tables are created
    void init();
    QVector<RawDatabase::Query> generateNewMessageQueries(const QString& friendPk, const QString& message,
                                    const QString& sender, const QDateTime &time, bool isSent, QString dispName,
                                                          std::function<void(int64_t)> insertIdCallback={});

private:
    RawDatabase db;
    // Cached mappings to speed up message saving
    QHash<QString, int64_t> peers; ///< Maps friend public keys to unique IDs by index
};

#endif // HISTORY_H
