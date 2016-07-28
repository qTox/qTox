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

class RawDatabase : QObject
{
    Q_OBJECT

public:
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
        QByteArray query;
        QVector<QByteArray> blobs;
        std::function<void(int64_t)> insertCallback;
        std::function<void(const QVector<QVariant>&)> rowCallback;
        QVector<sqlite3_stmt*> statements;

        friend class RawDatabase;
    };

public:
    RawDatabase(const QString& path, const QString& password);
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

protected:
    static QString deriveKey(const QString &password);
    static QVariant extractData(sqlite3_stmt* stmt, int col);

private:
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
    QString currentHexKey;
};

#endif // RAWDATABASE_H
