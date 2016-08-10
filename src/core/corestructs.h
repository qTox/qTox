#ifndef CORESTRUCTS_H
#define CORESTRUCTS_H

#include <QString>
#include <memory>

class QFile;
class QTimer;

enum class Status : int {Online = 0, Away, Busy, Offline};

struct DhtServer
{
    QString name;
    QString userId;
    QString address;
    quint16 port;
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

    ToxFile() = default;
    ToxFile(uint32_t FileNum, uint32_t FriendId, QByteArray FileName, QString filePath, FileDirection Direction);
    ~ToxFile(){}

    bool operator==(const ToxFile& other) const;
    bool operator!=(const ToxFile& other) const;

    void setFilePath(QString path);
    bool open(bool write);

    uint8_t fileKind;
    uint32_t fileNum;
    uint32_t friendId;
    QByteArray fileName;
    QString filePath;
    std::shared_ptr<QFile> file;
    quint64 bytesSent;
    quint64 filesize;
    FileStatus status;
    FileDirection direction;
    QByteArray avatarData;
    QByteArray resumeFileId;
};

#endif // CORESTRUCTS_H
