#include "src/core/corestructs.h"
#include "src/core/core.h"
#include <tox/tox.h>
#include <QFile>
#include <QRegularExpression>

#define TOX_HEX_ID_LENGTH 2*TOX_ADDRESS_SIZE

ToxFile::ToxFile(uint32_t fileNum, uint32_t friendId, QByteArray filename, QString filePath, FileDirection Direction)
    : fileKind{TOX_FILE_KIND_DATA}, fileNum(fileNum), friendId(friendId), fileName{filename},
      filePath{filePath}, file{new QFile(filePath)}, bytesSent{0}, filesize{0},
      status{STOPPED}, direction{Direction}
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
