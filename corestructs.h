#ifndef CORESTRUCTS_H
#define CORESTRUCTS_H

// Some headers use Core structs but don't need to include all of core.h
// They should include this file directly instead to reduce compilation times

#include <QString>
class QFile;
class QTimer;

enum class Status : int {Online = 0, Away, Busy, Offline};

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
        TRANSMITTING
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
