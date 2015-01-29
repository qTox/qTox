#ifndef CORESTRUCTS_H
#define CORESTRUCTS_H

// Some headers use Core structs but don't need to include all of core.h
// They should include this file directly instead to reduce compilation times

#include <QString>
class QFile;
class QTimer;

enum class Status : int {Online = 0, Away, Busy, Offline};

#define TOX_ID_PUBLIC_KEY_LENGTH 64
#define TOX_ID_NO_SPAM_LENGTH    8
#define TOX_ID_CHECKSUM_LENGTH   4

struct ToxID
{
    ToxID()=default;
    ToxID(const ToxID& other);

    QString publicKey;
    QString noSpam;
    QString checkSum;

    QString toString() const;
    static ToxID fromString(QString id);
    static bool isToxId(const QString& id);

    bool operator==(const ToxID& other) const;
    bool operator!=(const ToxID& other) const;
    bool isMine() const;
};

struct DhtServer
{
    QString name;
    QString userId;
    QString address;
    int port;
};

struct ToxFile
{
    enum FileStatus
    {
        STOPPED,
        PAUSED,
        TRANSMITTING,
        BROKEN
    };

    enum FileDirection : bool
    {
        SENDING,
        RECEIVING
    };

    ToxFile()=default;
    ToxFile(int FileNum, int FriendId, QByteArray FileName, QString FilePath, FileDirection Direction);
    ~ToxFile(){}
    void setFilePath(QString path);
    bool open(bool write);

    int fileNum;
    int friendId;
    QByteArray fileName;
    QString filePath;
    QFile* file;
    long long bytesSent;
    long long filesize;
    FileStatus status;
    FileDirection direction;
    QTimer* sendTimer;
};

#endif // CORESTRUCTS_H
