#ifndef CORESTRUCTS_H
#define CORESTRUCTS_H

#include <QString>
#include <memory>
#include <QCryptographicHash>

class QFile;
class QTimer;

struct ToxFile
{
    // Note do not change values, these are directly inserted into the DB in their
    // current form, changing order would mess up database state!
    enum FileStatus
    {
        INITIALIZING = 0,
        PAUSED = 1,
        TRANSMITTING = 2,
        BROKEN = 3,
        CANCELED = 4,
        FINISHED = 5,
    };

    // Note do not change values, these are directly inserted into the DB in their
    // current form (can add fields though as db representation is an int)
    enum FileDirection : bool
    {
        SENDING = 0,
        RECEIVING = 1,
    };

    ToxFile() = default;
    ToxFile(uint32_t FileNum, uint32_t FriendId, QByteArray FileName, QString filePath,
            FileDirection Direction);

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
    std::shared_ptr<QCryptographicHash> hashGenerator = std::make_shared<QCryptographicHash>(QCryptographicHash::Sha256);
};

#endif // CORESTRUCTS_H
