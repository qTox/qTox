#include "src/core/corestructs.h"
#include "src/core/core.h"
#include <tox/tox.h>
#include <QFile>
#include <QRegularExpression>

#define TOX_HEX_ID_LENGTH 2*TOX_ADDRESS_SIZE

/**
 * @file corestructs.h
 * @brief Some headers use Core structs but don't need to include all of core.h
 *
 * They should include this file directly instead to reduce compilation times
 *
 *
 * @var ToxFile::fileKind   Data file (default) or avatar
 */

/**
 * @brief   Compare equal operator
 * @param   other   the compared instance
 * @return  true, if equal; false otherwise
 */
bool DhtServer::operator==(const DhtServer& other) const
{
    return this == &other ||
            (port == other.port && address == other.address &&
             userId == other.userId && name == other.name);
}

/**
 * @brief   Compare not equal operator
 * @param   other   the compared instance
 * @return  true, if not equal; false otherwise
 */
bool DhtServer::operator!=(const DhtServer& other) const
{
    return !(*this == other);
}

/**
 * @brief ToxFile constructor
 */
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
