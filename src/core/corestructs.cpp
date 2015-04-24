#include "src/core/corestructs.h"
#include "src/core/core.h"
#include <tox/tox.h>
#include <QFile>
#include <QRegularExpression>

#define TOX_HEX_ID_LENGTH 2*TOX_ADDRESS_SIZE

ToxFile::ToxFile(uint32_t FileNum, uint32_t FriendId, QByteArray FileName, QString FilePath, FileDirection Direction)
    : fileNum(FileNum), friendId(FriendId), fileName{FileName}, filePath{FilePath}, file{new QFile(filePath)},
    bytesSent{0}, filesize{0}, status{STOPPED}, direction{Direction}, sendTimer{nullptr}
{
}

bool ToxFile::operator==(const ToxFile &other) const
{
    return (fileNum == other.fileNum) && (friendId == other.friendId) && (direction == other.direction);
}

bool ToxFile::operator!=(const ToxFile &other) const
{
    return !(*this == other);
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

ToxID::ToxID(const ToxID& other)
{
    publicKey = other.publicKey;
    noSpam = other.noSpam;
    checkSum = other.checkSum;
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
    toxID.checkSum  = id.mid(TOX_ID_PUBLIC_KEY_LENGTH + TOX_ID_NO_SPAM_LENGTH, TOX_ID_CHECKSUM_LENGTH);
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

void ToxID::clear()
{
    publicKey.clear();
}

bool ToxID::isToxId(const QString& value)
{
    const QRegularExpression hexRegExp("^[A-Fa-f0-9]+$");
    return value.length() == TOX_HEX_ID_LENGTH && value.contains(hexRegExp);
}
