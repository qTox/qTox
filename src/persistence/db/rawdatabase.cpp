#include "rawdatabase.h"
#include <QDebug>
#include <QMetaObject>
#include <QMutexLocker>
#include <QCoreApplication>
#include <QFile>
#include <cassert>
#include <tox/toxencryptsave.h>

/// The two following defines are required to use SQLCipher
/// They are used by the sqlite3.h header
#define SQLITE_HAS_CODEC
#define SQLITE_TEMP_STORE 2

#include <sqlcipher/sqlite3.h>

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

bool RawDatabase::isOpen()
{
    // We don't need thread safety since only the ctor/dtor can write this pointer
    return sqlite != nullptr;
}

bool RawDatabase::execNow(const QString& statement)
{
    return execNow(Query{statement});
}

bool RawDatabase::execNow(const RawDatabase::Query &statement)
{
    return execNow(QVector<Query>{statement});
}

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

void RawDatabase::sync()
{
    QMetaObject::invokeMethod(this, "process", Qt::BlockingQueuedConnection);
}

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


QString RawDatabase::deriveKey(QString password)
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
                    qWarning() << "Failed to prepare statement"<<query.query<<"with error"<<r;
                    goto cleanupStatements;
                }
                query.statements += stmt;

                // Now we can bind our params to this statement
                int nParams = sqlite3_bind_parameter_count(stmt);
                if (query.blobs.size() < curParam+nParams)
                {
                    qWarning() << "Not enough parameters to bind to query "<<query.query;
                    goto cleanupStatements;
                }
                for (int i=0; i<nParams; ++i)
                {
                    const QByteArray& blob = query.blobs[curParam+i];
                    if (sqlite3_bind_blob(stmt, i+1, blob.data(), blob.size(), SQLITE_STATIC) != SQLITE_OK)
                    {
                        qWarning() << "Failed to bind param"<<curParam+i<<"to query "<<query.query;
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
                        for (int i=0; i<column_count; ++i)
                            row += extractData(stmt, i);

                        query.rowCallback(row);
                    }
                } while (result == SQLITE_ROW);
                if (result == SQLITE_ERROR)
                {
                    qWarning() << "Error executing query "<<query.query;
                    goto cleanupStatements;
                }
                else if (result == SQLITE_MISUSE)
                {
                    qWarning() << "Misuse executing query "<<query.query;
                    goto cleanupStatements;
                }
                else if (result == SQLITE_CONSTRAINT)
                {
                    qWarning() << "Constraint error executing query "<<query.query;
                    goto cleanupStatements;
                }
                else if (result != SQLITE_DONE)
                {
                    qWarning() << "Unknown error"<<result<<"executing query "<<query.query;
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
