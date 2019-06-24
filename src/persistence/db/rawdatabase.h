/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#ifndef RAWDATABASE_H
#define RAWDATABASE_H

#include <QByteArray>
#include <QMutex>
#include <QPair>
#include <QQueue>
#include <QString>
#include <QThread>
#include <QVariant>
#include <QVector>
#include <QRegularExpression>
#include <atomic>
#include <functional>
#include <memory>
#include "src/util/strongtype.h"

/// The two following defines are required to use SQLCipher
/// They are used by the sqlite3.h header
#define SQLITE_HAS_CODEC
#define SQLITE_TEMP_STORE 2

#include <sqlite3.h>

using RowId = NamedType<int64_t, struct RowIdTag, Orderable>;
Q_DECLARE_METATYPE(RowId);

class RawDatabase : QObject
{
    Q_OBJECT

public:
    class Query
    {
    public:
        Query(QString query, QVector<QByteArray> blobs = {},
              const std::function<void(RowId)>& insertCallback = {})
            : query{query.toUtf8()}
            , blobs{blobs}
            , insertCallback{insertCallback}
        {
        }
        Query(QString query, const std::function<void(RowId)>& insertCallback)
            : query{query.toUtf8()}
            , insertCallback{insertCallback}
        {
        }
        Query(QString query, const std::function<void(const QVector<QVariant>&)>& rowCallback)
            : query{query.toUtf8()}
            , rowCallback{rowCallback}
        {
        }
        Query() = default;

    private:
        QByteArray query;
        QVector<QByteArray> blobs;
        std::function<void(RowId)> insertCallback;
        std::function<void(const QVector<QVariant>&)> rowCallback;
        QVector<sqlite3_stmt*> statements;

        friend class RawDatabase;
    };

public:
    RawDatabase(const QString& path, const QString& password, const QByteArray& salt);
    ~RawDatabase();
    bool isOpen();

    bool execNow(const QString& statement);
    bool execNow(const Query& statement);
    bool execNow(const QVector<Query>& statements);

    void execLater(const QString& statement);
    void execLater(const Query& statement);
    void execLater(const QVector<Query>& statements);

    void sync();

public slots:
    bool setPassword(const QString& password);
    bool rename(const QString& newPath);
    bool remove();

protected slots:
    bool open(const QString& path, const QString& hexKey = {});
    void close();
    void process();

private:
    QString anonymizeQuery(const QByteArray& query);

protected:
    static QString deriveKey(const QString& password, const QByteArray& salt);
    static QString deriveKey(const QString& password);
    static QVariant extractData(sqlite3_stmt* stmt, int col);
    static void regexpInsensitive(sqlite3_context* ctx, int argc, sqlite3_value** argv);
    static void regexpSensitive(sqlite3_context* ctx, int argc, sqlite3_value** argv);

private:
    static void regexp(sqlite3_context* ctx, int argc, sqlite3_value** argv, const QRegularExpression::PatternOptions cs);

    struct Transaction
    {
        QVector<Query> queries;
        std::atomic_bool* success = nullptr;
        std::atomic_bool* done = nullptr;
    };

private:
    sqlite3* sqlite;
    std::unique_ptr<QThread> workerThread;
    QQueue<Transaction> pendingTransactions;
    QMutex transactionsMutex;
    QString path;
    QByteArray currentSalt;
    QString currentHexKey;
};

#endif // RAWDATABASE_H
