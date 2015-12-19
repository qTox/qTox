#ifndef RAWDATABASE_H
#define RAWDATABASE_H

#include <QString>
#include <QByteArray>
#include <QThread>
#include <QQueue>
#include <QVector>
#include <QPair>
#include <QMutex>
#include <QVariant>
#include <memory>
#include <atomic>

struct sqlite3;
struct sqlite3_stmt;

/// Implements a low level RAII interface to a SQLCipher (SQlite3) database
/// Thread-safe, does all database operations on a worker thread
/// The queries must not contain transaction commands (BEGIN/COMMIT/...) or the behavior is undefined
class RawDatabase : QObject
{
    Q_OBJECT

public:
    /// A query to be executed by the database. Can be composed of one or more SQL statements in the query,
    /// optional BLOB parameters to be bound, and callbacks fired when the query is executed
    /// Calling any database method from a query callback is undefined behavior
    class Query
    {
    public:
        Query(QString query, QVector<QByteArray> blobs = {}, std::function<void(int64_t)> insertCallback={})
            : query{query.toUtf8()}, blobs{blobs}, insertCallback{insertCallback} {}
        Query(QString query, std::function<void(int64_t)> insertCallback)
            : query{query.toUtf8()}, insertCallback{insertCallback} {}
        Query(QString query, std::function<void(const QVector<QVariant>&)> rowCallback)
            : query{query.toUtf8()}, rowCallback{rowCallback} {}
        Query() = default;
    private:
        QByteArray query; ///< UTF-8 query string
        QVector<QByteArray> blobs; ///< Bound data blobs
        std::function<void(int64_t)> insertCallback; ///< Called after execution with the last insert rowid
        std::function<void(const QVector<QVariant>&)> rowCallback; ///< Called during execution for each row
        QVector<sqlite3_stmt*> statements; ///< Statements to be compiled from the query

        friend class RawDatabase;
    };

public:
    /// Tries to open a database
    /// If password is empty, the database will be opened unencrypted
    /// Otherwise we will use toxencryptsave to derive a key and encrypt the database
    RawDatabase(const QString& path, const QString& password);
    ~RawDatabase();
    bool isOpen(); ///< Returns true if the database was opened successfully
    /// Executes a SQL transaction synchronously.
    /// Returns whether the transaction was successful.
    bool execNow(const QString& statement);
    bool execNow(const Query& statement);
    bool execNow(const QVector<Query>& statements);
    /// Executes a SQL transaction asynchronously.
    void execLater(const QString& statement);
    void execLater(const Query& statement);
    void execLater(const QVector<Query>& statements);
    /// Waits until all the pending transactions are executed
    void sync();

public slots:
    /// Changes the database password, encrypting or decrypting if necessary
    /// If password is empty, the database will be decrypted
    /// Will process all transactions before changing the password
    bool setPassword(const QString& password);
    /// Moves the database file on disk to match the new path
    /// /// Will process all transactions before renaming
    bool rename(const QString& newPath);

protected slots:
    /// Should only be called from the constructor, runs on the caller's thread
    bool open(const QString& path, const QString& hexKey = {});
    /// Should only be called from the destructor, runs on the caller's thread
    void close();
    /// Implements the actual processing of pending transactions
    /// Unqueues, compiles, binds and executes queries, then notifies of results
    /// MUST only be called from the worker thread
    void process();
    /// Extracts a variant from one column of a result row depending on the column type
    QVariant extractData(sqlite3_stmt* stmt, int col);

protected:
    /// Derives a 256bit key from the password and returns it hex-encoded
    static QString deriveKey(QString password);

private:
    /// SQL transactions to be processed
    /// A transaction is made of queries, which can have bound BLOBs
    struct Transaction
    {
        QVector<Query> queries;
        /// If not a nullptr, the result of the transaction will be set
        std::atomic_bool* success = nullptr;
        /// If not a nullptr, will be set to true when the transaction has been executed
        std::atomic_bool* done = nullptr;
    };

private:
    sqlite3* sqlite;
    std::unique_ptr<QThread> workerThread;
    QQueue<Transaction> pendingTransactions;
    /// Protects pendingTransactions
    QMutex transactionsMutex;
    QString path;
    QString currentHexKey;
};

#endif // RAWDATABASE_H
