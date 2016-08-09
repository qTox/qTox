/*
    Copyright © 2014-2016 by The qTox Project

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

#include "rawdatabase.h"

#include <cassert>
#include <tox/toxencryptsave.h>

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QMetaObject>
#include <QMutexLocker>
#include <QRegularExpression>

/// The two following defines are required to use SQLCipher
/// They are used by the sqlite3.h header
#define SQLITE_HAS_CODEC
#define SQLITE_TEMP_STORE 2

#include <sqlite3.h>


/**
 * @class RawDatabase
 * @brief Implements a low level RAII interface to a SQLCipher (SQlite3) database.
 *
 * Thread-safe, does all database operations on a worker thread.
 * The queries must not contain transaction commands (BEGIN/COMMIT/...) or the behavior is undefined.
 *
 * @var QMutex RawDatabase::transactionsMutex;
 * @brief Protects pendingTransactions
 */

/**
 * @class Query
 * @brief A query to be executed by the database.
 *
 * Can be composed of one or more SQL statements in the query,
 * optional BLOB parameters to be bound, and callbacks fired when the query is executed
 * Calling any database method from a query callback is undefined behavior.
 *
 * @var QByteArray RawDatabase::Query::query
 * @brief UTF-8 query string
 *
 * @var QVector<QByteArray> RawDatabase::Query::blobs
 * @brief Bound data blobs
 *
 * @var std::function<void(int64_t)> RawDatabase::Query::insertCallback
 * @brief Called after execution with the last insert rowid
 *
 * @var std::function<void(const QVector<QVariant>&)> RawDatabase::Query::rowCallback
 * @brief Called during execution for each row
 *
 * @var QVector<sqlite3_stmt*> RawDatabase::Query::statements
 * @brief Statements to be compiled from the query
 */

/**
 * @struct Transaction
 * @brief SQL transactions to be processed.
 *
 * A transaction is made of queries, which can have bound BLOBs.
 *
 * @var std::atomic_bool* RawDatabase::Transaction::success = nullptr;
 * @brief If not a nullptr, the result of the transaction will be set
 *
 * @var std::atomic_bool* RawDatabase::Transaction::done = nullptr;
 * @brief If not a nullptr, will be set to true when the transaction has been executed
 */

/**
 * @brief Tries to open a database.
 * @param path Path to database.
 * @param password If empty, the database will be opened unencrypted.
 * Otherwise we will use toxencryptsave to derive a key and encrypt the database.
 */
RawDatabase::RawDatabase(const QString &path, const QString& password)
    : workerThread{new QThread}, path{path}, currentHexKey{deriveKey(password)}
{
    workerThread->setObjectName("qTox Database");
    moveToThread(workerThread.get());
    workerThread->start();

    if (!open(path, currentHexKey))
        return;
}

RawDatabase::~RawDatabase()
{
    close();
    workerThread->exit(0);
    while (workerThread->isRunning())
        workerThread->wait(50);
}

/**
 * @brief Tries to open the database with the given (possibly empty) key.
 * @param path Path to database.
 * @param hexKey Hex representation of the key in string.
 * @return True if success, false otherwise.
 */
bool RawDatabase::open(const QString& path, const QString &hexKey)
{
    if (QThread::currentThread() != workerThread.get())
    {
        bool ret;
        QMetaObject::invokeMethod(this, "open", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, ret),
                                  Q_ARG(const QString&, path), Q_ARG(const QString&, hexKey));
        return ret;
    }

    if (!QFile::exists(path) && QFile::exists(path+".tmp"))
    {
        qWarning() << "Restoring database from temporary export file! Did we crash while changing the password?";
        QFile::rename(path+".tmp", path);
    }

    if (sqlite3_open_v2(path.toUtf8().data(), &sqlite,
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, nullptr) != SQLITE_OK)
    {
        qWarning() << "Failed to open database"<<path<<"with error:"<<sqlite3_errmsg(sqlite);
        return false;
    }

    if (!hexKey.isEmpty())
    {
        if (!execNow("PRAGMA key = \"x'"+hexKey+"'\""))
        {
            qWarning() << "Failed to set encryption key";
            close();
            return false;
        }

        if (!execNow("SELECT count(*) FROM sqlite_master"))
        {
            qWarning() << "Database is unusable, check that the password is correct";
            close();
            return false;
        }
    }
    return true;
}

/**
 * @brief Close the database and free its associated resources.
 */
void RawDatabase::close()
{
    if (QThread::currentThread() != workerThread.get())
        return (void)QMetaObject::invokeMethod(this, "close", Qt::BlockingQueuedConnection);

    // We assume we're in the ctor or dtor, so we just need to finish processing our transactions
    process();

    if (sqlite3_close(sqlite) == SQLITE_OK)
        sqlite = nullptr;
    else
        qWarning() << "Error closing database:"<<sqlite3_errmsg(sqlite);
}

/**
 * @brief Checks, that the database is open.
 * @return True if the database was opened successfully.
 */
bool RawDatabase::isOpen()
{
    // We don't need thread safety since only the ctor/dtor can write this pointer
    return sqlite != nullptr;
}

/**
 * @brief Executes a SQL transaction synchronously.
 * @param statement Statement to execute.
 * @return Whether the transaction was successful.
 */
bool RawDatabase::execNow(const QString& statement)
{
    return execNow(Query{statement});
}

/**
 * @brief Executes a SQL transaction synchronously.
 * @param statement Statement to execute.
 * @return Whether the transaction was successful.
 */
bool RawDatabase::execNow(const RawDatabase::Query &statement)
{
    return execNow(QVector<Query>{statement});
}

/**
 * @brief Executes a SQL transaction synchronously.
 * @param statements List of statements to execute.
 * @return Whether the transaction was successful.
 */
bool RawDatabase::execNow(const QVector<RawDatabase::Query> &statements)
{
    if (!sqlite)
    {
        qWarning() << "Trying to exec, but the database is not open";
        return false;
    }

    std::atomic_bool done{false};
    std::atomic_bool success{false};

    Transaction trans;
    trans.queries = statements;
    trans.done = &done;
    trans.success = &success;
    {
        QMutexLocker locker{&transactionsMutex};
        pendingTransactions.enqueue(trans);
    }

    // We can't use blocking queued here, otherwise we might process future transactions
    // before returning, but we only want to wait until this transaction is done.
    QMetaObject::invokeMethod(this, "process");
    while (!done.load(std::memory_order_acquire))
        QThread::msleep(10);

    return success.load(std::memory_order_acquire);
}

/**
 * @brief Executes a SQL transaction asynchronously.
 * @param statement Statement to execute.
 */
void RawDatabase::execLater(const QString &statement)
{
    execLater(Query{statement});
}

void RawDatabase::execLater(const RawDatabase::Query &statement)
{
    execLater(QVector<Query>{statement});
}

void RawDatabase::execLater(const QVector<RawDatabase::Query> &statements)
{
    if (!sqlite)
    {
        qWarning() << "Trying to exec, but the database is not open";
        return;
    }

    Transaction trans;
    trans.queries = statements;
    {
        QMutexLocker locker{&transactionsMutex};
        pendingTransactions.enqueue(trans);
    }

    QMetaObject::invokeMethod(this, "process");
}

/**
 * @brief Waits until all the pending transactions are executed.
 */
void RawDatabase::sync()
{
    QMetaObject::invokeMethod(this, "process", Qt::BlockingQueuedConnection);
}

/**
 * @brief Changes the database password, encrypting or decrypting if necessary.
 * @param password If password is empty, the database will be decrypted.
 * @return True if success, false otherwise.
 * @note Will process all transactions before changing the password.
 */
bool RawDatabase::setPassword(const QString& password)
{
    if (!sqlite)
    {
        qWarning() << "Trying to change the password, but the database is not open";
        return false;
    }

    if (QThread::currentThread() != workerThread.get())
    {
        bool ret;
        QMetaObject::invokeMethod(this, "setPassword", Qt::BlockingQueuedConnection,
                                    Q_RETURN_ARG(bool, ret), Q_ARG(const QString&, password));
        return ret;
    }

    // If we need to decrypt or encrypt, we'll need to sync and close,
    // so we always process the pending queue before rekeying for consistency
    process();

    if (QFile::exists(path+".tmp"))
    {
        qWarning() << "Found old temporary export file while rekeying, deleting it";
        QFile::remove(path+".tmp");
    }

    if (!password.isEmpty())
    {
        QString newHexKey = deriveKey(password);
        if (!currentHexKey.isEmpty())
        {
            if (!execNow("PRAGMA rekey = \"x'"+newHexKey+"'\""))
            {
                qWarning() << "Failed to change encryption key";
                close();
                return false;
            }
        }
        else
        {
            // Need to encrypt the database
            if (!execNow("ATTACH DATABASE '"+path+".tmp' AS encrypted KEY \"x'"+newHexKey+"'\";"
                         "SELECT sqlcipher_export('encrypted');"
                         "DETACH DATABASE encrypted;"))
            {
                qWarning() << "Failed to export encrypted database";
                close();
                return false;
            }

            // This is racy as hell, but nobody will race with us since we hold the profile lock
            // If we crash or die here, the rename should be atomic, so we can recover no matter what
            close();
            QFile::remove(path);
            QFile::rename(path+".tmp", path);
            currentHexKey = newHexKey;
            if (!open(path, currentHexKey))
            {
                qWarning() << "Failed to open encrypted database";
                return false;
            }
        }
    }
    else
    {
        if (currentHexKey.isEmpty())
            return true;

        // Need to decrypt the database
        if (!execNow("ATTACH DATABASE '"+path+".tmp' AS plaintext KEY '';"
                     "SELECT sqlcipher_export('plaintext');"
                     "DETACH DATABASE plaintext;"))
        {
            qWarning() << "Failed to export decrypted database";
            close();
            return false;
        }

        // This is racy as hell, but nobody will race with us since we hold the profile lock
        // If we crash or die here, the rename should be atomic, so we can recover no matter what
        close();
        QFile::remove(path);
        QFile::rename(path+".tmp", path);
        currentHexKey.clear();
        if (!open(path))
        {
            qCritical() << "Failed to open decrypted database";
            return false;
        }
    }
    return true;
}

/**
 * @brief Moves the database file on disk to match the new path.
 * @param newPath Path to move database file.
 * @return True if success, false otherwise.
 *
 * @note Will process all transactions before renaming
 */
bool RawDatabase::rename(const QString &newPath)
{
    if (!sqlite)
    {
        qWarning() << "Trying to change the password, but the database is not open";
        return false;
    }

    if (QThread::currentThread() != workerThread.get())
    {
        bool ret;
        QMetaObject::invokeMethod(this, "rename", Qt::BlockingQueuedConnection,
                                    Q_RETURN_ARG(bool, ret), Q_ARG(const QString&, newPath));
        return ret;
    }

    process();

    if (path == newPath)
        return true;

    if (QFile::exists(newPath))
        return false;

    close();
    if (!QFile::rename(path, newPath))
        return false;
    path = newPath;
    return open(path, currentHexKey);
}

/**
 * @brief Deletes the on disk database file after closing it.
 * @note Will process all transactions before deletings.
 * @return True if success, false otherwise.
 */
bool RawDatabase::remove()
{
    if (!sqlite)
    {
        qWarning() << "Trying to remove the database, but it is not open";
        return false;
    }

    if (QThread::currentThread() != workerThread.get())
    {
        bool ret;
        QMetaObject::invokeMethod(this, "remove", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, ret));
        return ret;
    }

    qDebug() << "Removing database "<< path;
    close();
    return QFile::remove(path);
}

/**
 * @brief Derives a 256bit key from the password and returns it hex-encoded
 * @param password Password to decrypt database
 * @return String representation of key
 */
QString RawDatabase::deriveKey(const QString &password)
{
    if (password.isEmpty())
        return {};

    QByteArray passData = password.toUtf8();

    static_assert(TOX_PASS_KEY_LENGTH >= 32, "toxcore must provide 256bit or longer keys");

    static const uint8_t expandConstant[TOX_PASS_SALT_LENGTH+1] = "L'ignorance est le pire des maux";
    TOX_PASS_KEY key;
    tox_derive_key_with_salt((uint8_t*)passData.data(), passData.size(), expandConstant, &key, nullptr);
    return QByteArray((char*)key.key, 32).toHex();
}

/**
 * @brief Implements the actual processing of pending transactions.
 * Unqueues, compiles, binds and executes queries, then notifies of results
 *
 * @warning MUST only be called from the worker thread
 */
void RawDatabase::process()
{
    assert(QThread::currentThread() == workerThread.get());

    if (!sqlite)
        return;

    forever
    {
        // Fetch the next transaction
        Transaction trans;
        {
            QMutexLocker locker{&transactionsMutex};
            if (pendingTransactions.isEmpty())
                return;
            trans = pendingTransactions.dequeue();
        }

        // In case we exit early, prepare to signal errors
        if (trans.success != nullptr)
            trans.success->store(false, std::memory_order_release);

        // Add transaction commands if necessary
        if (trans.queries.size() > 1)
        {
            trans.queries.prepend({"BEGIN;"});
            trans.queries.append({"COMMIT;"});
        }

        // Compile queries
        for (Query& query : trans.queries)
        {
            assert(query.statements.isEmpty());
            // sqlite3_prepare_v2 only compiles one statement at a time in the query, we need to loop over them all
            int curParam=0;
            const char* compileTail = query.query.data();
            do {
                // Compile the next statement
                sqlite3_stmt* stmt;
                int r;
                if ((r = sqlite3_prepare_v2(sqlite, compileTail,
                                       query.query.size() - static_cast<int>(compileTail - query.query.data()),
                                       &stmt, &compileTail)) != SQLITE_OK)
                {
                    qWarning() << "Failed to prepare statement" << anonymizeQuery(query.query)
                               << "with error" << r;
                    goto cleanupStatements;
                }
                query.statements += stmt;

                // Now we can bind our params to this statement
                int nParams = sqlite3_bind_parameter_count(stmt);
                if (query.blobs.size() < curParam+nParams)
                {
                    qWarning() << "Not enough parameters to bind to query "
                               << anonymizeQuery(query.query);
                    goto cleanupStatements;
                }
                for (int i=0; i<nParams; ++i)
                {
                    const QByteArray& blob = query.blobs[curParam+i];
                    if (sqlite3_bind_blob(stmt, i+1, blob.data(), blob.size(), SQLITE_STATIC) != SQLITE_OK)
                    {
                        qWarning() << "Failed to bind param" << curParam + i
                                   << "to query" << anonymizeQuery(query.query);
                        goto cleanupStatements;
                    }
                }
                curParam += nParams;
            } while (compileTail != query.query.data()+query.query.size());
        }

        // Execute each statement of each query of our transaction
        for (Query& query : trans.queries)
        {
            for (sqlite3_stmt* stmt : query.statements)
            {
                int column_count = sqlite3_column_count(stmt);
                int result;
                do {
                    result = sqlite3_step(stmt);

                    // Execute our row callback
                    if (result == SQLITE_ROW && query.rowCallback)
                    {
                        QVector<QVariant> row;
                        for (int i = 0; i < column_count; ++i)
                            row += extractData(stmt, i);

                        query.rowCallback(row);
                    }
                } while (result == SQLITE_ROW);

                if (result == SQLITE_DONE)
                    continue;

                QString anonQuery = anonymizeQuery(query.query);
                switch (result) {
                case SQLITE_ERROR:
                    qWarning() << "Error executing query" << anonQuery;
                    goto cleanupStatements;
                case SQLITE_MISUSE:
                    qWarning() << "Misuse executing query" << anonQuery;
                    goto cleanupStatements;
                case SQLITE_CONSTRAINT:
                    qWarning() << "Constraint error executing query" << anonQuery;
                    goto cleanupStatements;
                default:
                    qWarning() << "Unknown error" << result
                               << "executing query" << anonQuery;
                    goto cleanupStatements;
                }
            }

            if (query.insertCallback)
                query.insertCallback(sqlite3_last_insert_rowid(sqlite));
        }

        if (trans.success != nullptr)
            trans.success->store(true, std::memory_order_release);

        // Free our statements
        cleanupStatements:
        for (Query& query : trans.queries)
        {
            for (sqlite3_stmt* stmt : query.statements)
                sqlite3_finalize(stmt);
            query.statements.clear();
        }

        // Signal transaction results
        if (trans.done != nullptr)
            trans.done->store(true, std::memory_order_release);
    }
}

/**
 * @brief Hides public keys and timestamps in query.
 * @param query Source query, which should be anonymized.
 * @return Query without timestamps and public keys.
 */
QString RawDatabase::anonymizeQuery(const QByteArray& query)
{
    QString queryString(query);
    queryString.replace(QRegularExpression("chat.public_key='[A-F0-9]{64}'"),
                        "char.public_key='<HERE IS PUBLIC KEY>'");
    queryString.replace(QRegularExpression("timestamp BETWEEN \\d{5,} AND \\d{5,}"),
                        "timestamp BETWEEN <START HERE> AND <END HERE>");

    return queryString;
}

/**
 * @brief Extracts a variant from one column of a result row depending on the column type.
 * @param stmt Statement to execute.
 * @param col Number of column to extract.
 * @return Extracted data.
 */
QVariant RawDatabase::extractData(sqlite3_stmt *stmt, int col)
{
    int type = sqlite3_column_type(stmt, col);
    if (type == SQLITE_INTEGER)
    {
        return sqlite3_column_int64(stmt, col);
    }
    else if (type == SQLITE_TEXT)
    {
        const char* str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col));
        int len = sqlite3_column_bytes(stmt, col);
        return QString::fromUtf8(str, len);
    }
    else if (type == SQLITE_NULL)
    {
        return QVariant{};
    }
    else
    {
        const char* data = reinterpret_cast<const char*>(sqlite3_column_blob(stmt, col));
        int len = sqlite3_column_bytes(stmt, col);
        return QByteArray::fromRawData(data, len);
    }
}
