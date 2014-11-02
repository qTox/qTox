#include "src/corestructs.h"
#include "src/core.h"
#include <QFile>

ToxFile::ToxFile(int FileNum, int FriendId, QByteArray FileName, QString FilePath, FileDirection Direction)
    : fileNum(FileNum), friendId(FriendId), fileName{FileName}, filePath{FilePath}, file{new QFile(filePath)},
    bytesSent{0}, filesize{0}, status{STOPPED}, direction{Direction}, sendTimer{nullptr}
{
}

void ToxFile::setFilePath(QString path)
{
    filePath=path;
    file->setFileName(path);
}

bool ToxFile::open(bool write)
{
    return write ? file->open(QIODevice::ReadWrite) : file->open(QIODevice::ReadOnly);
}

QString ToxID::toString() const
{
    return publicKey + noSpam + checkSum;
}

ToxID ToxID::fromString(QString id)
{
    ToxID toxID;
    toxID.publicKey = id.left(TOX_ID_PUBLIC_KEY_LENGTH);
    toxID.noSpam    = id.mid(TOX_ID_PUBLIC_KEY_LENGTH, TOX_ID_NO_SPAM_LENGTH);
    toxID.checkSum  = id.right(TOX_ID_CHECKSUM_LENGTH);
    return toxID;
}


bool ToxID::operator==(const ToxID& other) const
{
    return publicKey == other.publicKey;
}

bool ToxID::operator!=(const ToxID& other) const
{
    return publicKey != other.publicKey;
}

bool ToxID::isMine() const
{
    return *this == Core::getInstance()->getSelfId();
}
