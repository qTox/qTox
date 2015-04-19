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
    void clear();
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
    ToxFile(uint32_t FileNum, uint32_t FriendId, QByteArray FileName, QString FilePath, FileDirection Direction);
    ~ToxFile(){}

    bool operator==(const ToxFile& other) const;
    bool operator!=(const ToxFile& other) const;

    void setFilePath(QString path);
    bool open(bool write);

    uint32_t fileNum;
    uint32_t friendId;
    QByteArray fileName;
    QString filePath;
    QFile* file;
    qint64 bytesSent;
    qint64 filesize;
    FileStatus status;
    FileDirection direction;
    QTimer* sendTimer;
};

#endif // CORESTRUCTS_H
