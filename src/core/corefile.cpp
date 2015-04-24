#include "core.h"
#include "corefile.h"
#include "corestructs.h"
#include "src/misc/cstring.h"
#include <QDebug>
#include <QFile>
#include <QThread>
#include <memory>

QMutex CoreFile::fileSendMutex;
QHash<uint64_t, ToxFile> CoreFile::fileMap;
using namespace std;

void CoreFile::sendFile(Core* core, uint32_t friendId, QString Filename, QString FilePath, long long filesize)
{
    QMutexLocker mlocker(&fileSendMutex);

    QByteArray fileName = Filename.toUtf8();
    uint32_t fileNum = tox_file_send(core->tox, friendId, TOX_FILE_KIND_DATA, filesize, nullptr,
                                (uint8_t*)fileName.data(), fileName.size(), nullptr);
    if (fileNum == UINT32_MAX)
    {
        qWarning() << "CoreFile::sendFile: Can't create the Tox file sender";
        emit core->fileSendFailed(friendId, Filename);
        return;
    }
    qDebug() << QString("CoreFile::sendFile: Created file sender %1 with friend %2").arg(fileNum).arg(friendId);

    ToxFile file{fileNum, friendId, fileName, FilePath, ToxFile::SENDING};
    file.filesize = filesize;
    if (!file.open(false))
    {
        qWarning() << QString("CoreFile::sendFile: Can't open file, error: %1").arg(file.file->errorString());
    }
    addFile(friendId, fileNum, file);

    emit core->fileSendStarted(file);
}

void CoreFile::pauseResumeFileSend(Core* core, uint32_t friendId, uint32_t fileId)
{
    ToxFile* file = findFile(friendId, fileId);
    if (!file)
    {
        qWarning("CoreFile::pauseResumeFileSend: No such file in queue");
        return;
    }
    if (file->status == ToxFile::TRANSMITTING)
    {
        file->status = ToxFile::PAUSED;
        emit core->fileTransferPaused(*file);
        tox_file_control(core->tox, file->friendId, file->fileNum, TOX_FILE_CONTROL_PAUSE, nullptr);
    }
    else if (file->status == ToxFile::PAUSED)
    {
        file->status = ToxFile::TRANSMITTING;
        emit core->fileTransferAccepted(*file);
        tox_file_control(core->tox, file->friendId, file->fileNum, TOX_FILE_CONTROL_RESUME, nullptr);
    }
    else
        qWarning() << "CoreFile::pauseResumeFileSend: File is stopped";
}

void CoreFile::pauseResumeFileRecv(Core* core, uint32_t friendId, uint32_t fileId)
{
    ToxFile* file = findFile(friendId, fileId);
    if (!file)
    {
        qWarning("CoreFile::cancelFileRecv: No such file in queue");
        return;
    }
    if (file->status == ToxFile::TRANSMITTING)
    {
        file->status = ToxFile::PAUSED;
        emit core->fileTransferPaused(*file);
        tox_file_control(core->tox, file->friendId, file->fileNum, TOX_FILE_CONTROL_PAUSE, nullptr);
    }
    else if (file->status == ToxFile::PAUSED)
    {
        file->status = ToxFile::TRANSMITTING;
        emit core->fileTransferAccepted(*file);
        tox_file_control(core->tox, file->friendId, file->fileNum, TOX_FILE_CONTROL_RESUME, nullptr);
    }
    else
        qWarning() << "CoreFile::pauseResumeFileRecv: File is stopped or broken";
}

void CoreFile::cancelFileSend(Core* core, uint32_t friendId, uint32_t fileId)
{
    ToxFile* file = findFile(friendId, fileId);
    if (!file)
    {
        qWarning("CoreFile::cancelFileSend: No such file in queue");
        return;
    }
    file->status = ToxFile::STOPPED;
    emit core->fileTransferCancelled(*file);
    tox_file_control(core->tox, file->friendId, file->fileNum, TOX_FILE_CONTROL_CANCEL, nullptr);
    while (file->sendTimer) QThread::msleep(1); // Wait until sendAllFileData returns before deleting
    removeFile(friendId, fileId);
}

void CoreFile::cancelFileRecv(Core* core, uint32_t friendId, uint32_t fileId)
{
    ToxFile* file = findFile(friendId, fileId);
    if (!file)
    {
        qWarning("CoreFile::cancelFileRecv: No such file in queue");
        return;
    }
    file->status = ToxFile::STOPPED;
    emit core->fileTransferCancelled(*file);
    tox_file_control(core->tox, file->friendId, file->fileNum, TOX_FILE_CONTROL_CANCEL, nullptr);
    removeFile(friendId, fileId);
}

void CoreFile::rejectFileRecvRequest(Core* core, uint32_t friendId, uint32_t fileId)
{
    ToxFile* file = findFile(friendId, fileId);
    if (!file)
    {
        qWarning("CoreFile::rejectFileRecvRequest: No such file in queue");
        return;
    }
    file->status = ToxFile::STOPPED;
    emit core->fileTransferCancelled(*file);
    tox_file_control(core->tox, file->friendId, file->fileNum, TOX_FILE_CONTROL_CANCEL, nullptr);
    removeFile(friendId, fileId);
}

void CoreFile::acceptFileRecvRequest(Core* core, uint32_t friendId, uint32_t fileId, QString path)
{
    ToxFile* file = findFile(friendId, fileId);
    if (!file)
    {
        qWarning("CoreFile::acceptFileRecvRequest: No such file in queue");
        return;
    }
    file->setFilePath(path);
    if (!file->open(true))
    {
        qWarning() << "CoreFile::acceptFileRecvRequest: Unable to open file";
        return;
    }
    file->status = ToxFile::TRANSMITTING;
    emit core->fileTransferAccepted(*file);
    tox_file_control(core->tox, file->friendId, file->fileNum, TOX_FILE_CONTROL_RESUME, nullptr);
}

ToxFile* CoreFile::findFile(uint32_t friendId, uint32_t fileId)
{
    uint64_t key = ((uint64_t)friendId<<32) + (uint64_t)fileId;
    if (!fileMap.contains(key))
    {
        qWarning() << "CoreFile::addFile: File transfer with ID "<<friendId<<':'<<fileId<<" doesn't exist";
        return nullptr;
    }
    else
        return &fileMap[key];
}

void CoreFile::addFile(uint32_t friendId, uint32_t fileId, const ToxFile& file)
{
    uint64_t key = ((uint64_t)friendId<<32) + (uint64_t)fileId;
    if (fileMap.contains(key))
        qWarning() << "CoreFile::addFile: Overwriting existing file transfer with same ID "<<friendId<<':'<<fileId;
    fileMap.insert(key, file);
}

void CoreFile::removeFile(uint32_t friendId, uint32_t fileId)
{
    uint64_t key = ((uint64_t)friendId<<32) + (uint64_t)fileId;
    if (!fileMap.remove(key))
        qWarning() << "CoreFile::removeFile: No such file in queue";
}

void CoreFile::onFileReceiveCallback(Tox*, uint32_t friendId, uint32_t fileId, uint32_t kind,
                                 uint64_t filesize, const uint8_t *fname, size_t fnameLen, void *core)
{
    qDebug() << QString("CoreFile: Received file request %1:%2 kind %3")
                        .arg(friendId).arg(fileId).arg(kind);

    ToxFile file{fileId, friendId,
                CString::toString(fname,fnameLen).toUtf8(), "", ToxFile::RECEIVING};
    file.filesize = filesize;
    file.fileKind = kind;
    addFile(friendId, fileId, file);
    if (kind == TOX_FILE_KIND_DATA)
        emit static_cast<Core*>(core)->fileReceiveRequested(file);
}
void CoreFile::onFileControlCallback(Tox*, uint32_t friendId, uint32_t fileId,
                                 TOX_FILE_CONTROL control, void *core)
{
    ToxFile* file = findFile(friendId, fileId);
    if (!file)
    {
        qWarning("CoreFile::onFileControlCallback: No such file in queue");
        return;
    }

    if (control == TOX_FILE_CONTROL_CANCEL)
    {
        qDebug() << "CoreFile::onFileControlCallback: Received cancel for file "<<friendId<<":"<<fileId;
        emit static_cast<Core*>(core)->fileTransferCancelled(*file);
        removeFile(friendId, fileId);
    }
    else if (control == TOX_FILE_CONTROL_PAUSE)
    {
        qDebug() << "CoreFile::onFileControlCallback: Received pause for file "<<friendId<<":"<<fileId;
        file->status = ToxFile::PAUSED;
        emit static_cast<Core*>(core)->fileTransferRemotePausedUnpaused(*file, true);
    }
    else if (control == TOX_FILE_CONTROL_RESUME)
    {
        qDebug() << "CoreFile::onFileControlCallback: Received pause for file "<<friendId<<":"<<fileId;
        file->status = ToxFile::TRANSMITTING;
        emit static_cast<Core*>(core)->fileTransferRemotePausedUnpaused(*file, false);
    }
    else
    {
        qWarning() << "Unhandled file control "<<control<<" for file "<<friendId<<':'<<fileId;
    }
}

void CoreFile::onFileDataCallback(Tox *tox, uint32_t friendId, uint32_t fileId,
                              uint64_t pos, size_t length, void* core)
{
    //qDebug() << "File data req of "<<length<<" at "<<pos<<" for file "<<friendId<<':'<<fileId;

    ToxFile* file = findFile(friendId, fileId);
    if (!file)
    {
        qWarning("CoreFile::onFileDataCallback: No such file in queue");
        return;
    }

    // If we reached EOF, ack and cleanup the transfer
    if (!length)
    {
        qDebug("CoreFile::onFileDataCallback: File sending completed");
        emit static_cast<Core*>(core)->fileTransferFinished(*file);
        removeFile(friendId, fileId);
        return;
    }

    unique_ptr<uint8_t[]> data(new uint8_t[length]);

    file->file->seek(pos);
    int64_t nread = file->file->read((char*)data.get(), length);
    if (nread <= 0)
    {
        qWarning("CoreFile::onFileDataCallback: Failed to read from file");
        emit static_cast<Core*>(core)->fileTransferCancelled(*file);
        tox_file_send_chunk(tox, friendId, fileId, pos, nullptr, 0, nullptr);
        removeFile(friendId, fileId);
        return;
    }
    file->bytesSent += length;

    if (!tox_file_send_chunk(tox, friendId, fileId, pos, data.get(), nread, nullptr))
    {
        qWarning("CoreFile::onFileDataCallback: Failed to send data chunk");
        return;
    }
    emit static_cast<Core*>(core)->fileTransferInfo(*file);
}

void CoreFile::onFileRecvChunkCallback(Tox *tox, uint32_t friendId, uint32_t fileId, uint64_t position,
                                    const uint8_t *data, size_t length, void *core)
{
    //qDebug() << QString("CoreFile: Received file chunk for request %1:%2").arg(friendId).arg(fileId);

    ToxFile* file = findFile(friendId, fileId);
    if (!file)
    {
        qWarning("CoreFile::onFileRecvChunkCallback: No such file in queue");
        tox_file_control(tox, friendId, fileId, TOX_FILE_CONTROL_CANCEL, nullptr);
        return;
    }

    if (file->bytesSent != position)
    {
        /// TODO: Allow ooo receiving for non-stream transfers, with very careful checking
        qWarning("CoreFile::onFileRecvChunkCallback: Received a chunk out-of-order, aborting transfer");
        emit static_cast<Core*>(core)->fileTransferCancelled(*file);
        tox_file_control(tox, friendId, fileId, TOX_FILE_CONTROL_CANCEL, nullptr);
        removeFile(friendId, fileId);
        return;
    }
    file->bytesSent += length;
    file->file->write((char*)data,length);
    //qDebug() << QString("CoreFile::onFileRecvChunkCallback: received %1/%2 bytes").arg(file->bytesSent).arg(file->filesize);

    if (file->bytesSent == file->filesize)
        emit static_cast<Core*>(core)->fileTransferFinished(*file);
    else
        emit static_cast<Core*>(core)->fileTransferInfo(*file);
}
