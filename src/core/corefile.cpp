#include "corefile.h"
#include <QDebug>

void CoreFile::onFileReceiveCallback(Tox*, uint32_t friendId, uint32_t fileId, uint32_t kind,
                                 uint64_t filesize, const uint8_t *fname, size_t fnameLen, void *core)
{
    qDebug() << QString("Core: Received file request %1:%2").arg(friendId).arg(fileId);

}
void CoreFile::onFileControlCallback(Tox* tox, uint32_t friendnumber, uint32_t filenumber,
                                 TOX_FILE_CONTROL control, void *core)
{
    qDebug() << "File control "<<control<<" for file "<<friendnumber<<':'<<filenumber;
}

void CoreFile::onFileDataCallback(Tox *tox, uint32_t friendnumber, uint32_t filenumber,
                              uint64_t pos, size_t length, void *core)
{
    qDebug() << "File data req of "<<length<<" at "<<pos<<" for file "<<friendnumber<<':'<<filenumber;
}

void CoreFile::onFileRecvChunkCallback(Tox *tox, uint32_t friendId, uint32_t fileId, uint64_t position,
                                    const uint8_t *data, size_t length, void *core)
{
    qDebug() << QString("Core: Received file chunk for request %1:%2").arg(friendId).arg(fileId);
}
