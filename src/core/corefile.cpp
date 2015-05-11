#include "core.h"
#include "corefile.h"
#include "corestructs.h"
#include "src/misc/cstring.h"
#include "src/misc/settings.h"
#include <QDebug>
#include <QFile>
#include <QThread>
#include <QDir>
#include <memory>

QMutex CoreFile::fileSendMutex;
QHash<uint64_t, ToxFile> CoreFile::fileMap;
using namespace std;

void CoreFile::sendAvatarFile(Core* core, uint32_t friendId, const QByteArray& data)
{
    QMutexLocker mlocker(&fileSendMutex);

    uint8_t filename[TOX_HASH_LENGTH];
    tox_hash(filename, (uint8_t*)data.data(), data.size());
    uint64_t filesize = data.size();
    uint32_t fileNum = tox_file_send(core->tox, friendId, TOX_FILE_KIND_AVATAR, filesize,
                                     nullptr, filename, TOX_HASH_LENGTH, nullptr);
    if (fileNum == std::numeric_limits<uint32_t>::max())
    {
        qWarning() << "sendAvatarFile: Can't create the Tox file sender";
        return;
    }
    //qDebug() << QString("sendAvatarFile: Created file sender %1 with friend %2").arg(fileNum).arg(friendId);

    ToxFile file{fileNum, friendId, "", "", ToxFile::SENDING};
    file.filesize = filesize;
    file.fileName = QByteArray((char*)filename, TOX_HASH_LENGTH);
    file.fileKind = TOX_FILE_KIND_AVATAR;
    file.avatarData = data;
    file.resumeFileId.resize(TOX_FILE_ID_LENGTH);
    tox_file_get_file_id(core->tox, friendId, fileNum, (uint8_t*)file.resumeFileId.data(), nullptr);
    addFile(friendId, fileNum, file);
}

void CoreFile::sendFile(Core* core, uint32_t friendId, QString Filename, QString FilePath, long long filesize)
{
    QMutexLocker mlocker(&fileSendMutex);

    QByteArray fileName = Filename.toUtf8();
    uint32_t fileNum = tox_file_send(core->tox, friendId, TOX_FILE_KIND_DATA, filesize, nullptr,
                                (uint8_t*)fileName.data(), fileName.size(), nullptr);
    if (fileNum == std::numeric_limits<uint32_t>::max())
    {
        qWarning() << "sendFile: Can't create the Tox file sender";
        emit core->fileSendFailed(friendId, Filename);
        return;
    }
    qDebug() << QString("sendFile: Created file sender %1 with friend %2").arg(fileNum).arg(friendId);

    ToxFile file{fileNum, friendId, fileName, FilePath, ToxFile::SENDING};
    file.filesize = filesize;
    file.resumeFileId.resize(TOX_FILE_ID_LENGTH);
    tox_file_get_file_id(core->tox, friendId, fileNum, (uint8_t*)file.resumeFileId.data(), nullptr);
    if (!file.open(false))
    {
        qWarning() << QString("sendFile: Can't open file, error: %1").arg(file.file->errorString());
    }
    addFile(friendId, fileNum, file);

    emit core->fileSendStarted(file);
}

void CoreFile::pauseResumeFileSend(Core* core, uint32_t friendId, uint32_t fileId)
{
    ToxFile* file = findFile(friendId, fileId);
    if (!file)
    {
        qWarning("pauseResumeFileSend: No such file in queue");
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
        qWarning() << "pauseResumeFileSend: File is stopped";
}

void CoreFile::pauseResumeFileRecv(Core* core, uint32_t friendId, uint32_t fileId)
{
    ToxFile* file = findFile(friendId, fileId);
    if (!file)
    {
        qWarning("cancelFileRecv: No such file in queue");
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
        qWarning() << "pauseResumeFileRecv: File is stopped or broken";
}

void CoreFile::cancelFileSend(Core* core, uint32_t friendId, uint32_t fileId)
{
    ToxFile* file = findFile(friendId, fileId);
    if (!file)
    {
        qWarning("cancelFileSend: No such file in queue");
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
        qWarning("cancelFileRecv: No such file in queue");
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
        qWarning("rejectFileRecvRequest: No such file in queue");
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
        qWarning("acceptFileRecvRequest: No such file in queue");
        return;
    }
    file->setFilePath(path);
    if (!file->open(true))
    {
        qWarning() << "acceptFileRecvRequest: Unable to open file";
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
        qWarning() << "findFile: File transfer with ID "<<friendId<<':'<<fileId<<" doesn't exist";
        return nullptr;
    }
    else
        return &fileMap[key];
}

void CoreFile::addFile(uint32_t friendId, uint32_t fileId, const ToxFile& file)
{
    uint64_t key = ((uint64_t)friendId<<32) + (uint64_t)fileId;
    if (fileMap.contains(key))
        qWarning() << "addFile: Overwriting existing file transfer with same ID "<<friendId<<':'<<fileId;
    fileMap.insert(key, file);
}

void CoreFile::removeFile(uint32_t friendId, uint32_t fileId)
{
    uint64_t key = ((uint64_t)friendId<<32) + (uint64_t)fileId;
    if (!fileMap.contains(key))
    {
        qWarning() << "removeFile: No such file in queue";
        return;
    }
    fileMap[key].file->close();
    fileMap.remove(key);
}

void CoreFile::onFileReceiveCallback(Tox*, uint32_t friendId, uint32_t fileId, uint32_t kind,
                                 uint64_t filesize, const uint8_t *fname, size_t fnameLen, void *_core)
{
    Core* core = static_cast<Core*>(_core);
    qDebug() << QString("Received file request %1:%2 kind %3")
                        .arg(friendId).arg(fileId).arg(kind);

    if (kind == TOX_FILE_KIND_AVATAR)
    {
        QString friendAddr = core->getFriendAddress(friendId);
        if (!filesize)
        {
            // Avatars of size 0 means explicitely no avatar
            emit core->friendAvatarRemoved(friendId);
            QFile::remove(QDir(Settings::getSettingsDirPath()).filePath("avatars/"+friendAddr.left(64)+".png"));
            QFile::remove(QDir(Settings::getSettingsDirPath()).filePath("avatars/"+friendAddr.left(64)+".hash"));
            return;
        }
        else if (Settings::getInstance().getAvatarHash(friendAddr) == QByteArray((char*)fname, fnameLen))
        {
            // If it's an avatar but we already have it cached, cancel
            tox_file_control(core->tox, friendId, fileId, TOX_FILE_CONTROL_CANCEL, nullptr);
            return;
        }
        else
        {
            // It's an avatar and we don't have it, autoaccept the transfer
            tox_file_control(core->tox, friendId, fileId, TOX_FILE_CONTROL_RESUME, nullptr);
        }
    }

    ToxFile file{fileId, friendId,
                CString::toString(fname,fnameLen).toUtf8(), "", ToxFile::RECEIVING};
    file.filesize = filesize;
    file.fileKind = kind;
    file.resumeFileId.resize(TOX_FILE_ID_LENGTH);
    tox_file_get_file_id(core->tox, friendId, fileId, (uint8_t*)file.resumeFileId.data(), nullptr);
    addFile(friendId, fileId, file);
    if (kind != TOX_FILE_KIND_AVATAR)
        emit core->fileReceiveRequested(file);
}
void CoreFile::onFileControlCallback(Tox*, uint32_t friendId, uint32_t fileId,
                                 TOX_FILE_CONTROL control, void *core)
{
    ToxFile* file = findFile(friendId, fileId);
    if (!file)
    {
        qWarning("onFileControlCallback: No such file in queue");
        return;
    }

    if (control == TOX_FILE_CONTROL_CANCEL)
    {
        qDebug() << "onFileControlCallback: Received cancel for file "<<friendId<<":"<<fileId;
        emit static_cast<Core*>(core)->fileTransferCancelled(*file);
        removeFile(friendId, fileId);
    }
    else if (control == TOX_FILE_CONTROL_PAUSE)
    {
        qDebug() << "onFileControlCallback: Received pause for file "<<friendId<<":"<<fileId;
        file->status = ToxFile::PAUSED;
        emit static_cast<Core*>(core)->fileTransferRemotePausedUnpaused(*file, true);
    }
    else if (control == TOX_FILE_CONTROL_RESUME)
    {
        qDebug() << "onFileControlCallback: Received pause for file "<<friendId<<":"<<fileId;
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
        qWarning("onFileDataCallback: No such file in queue");
        return;
    }

    // If we reached EOF, ack and cleanup the transfer
    if (!length)
    {
        //qDebug("onFileDataCallback: File sending completed");
        if (file->fileKind != TOX_FILE_KIND_AVATAR)
            emit static_cast<Core*>(core)->fileTransferFinished(*file);
        removeFile(friendId, fileId);
        return;
    }

    unique_ptr<uint8_t[]> data(new uint8_t[length]);
    int64_t nread;

    if (file->fileKind == TOX_FILE_KIND_AVATAR)
    {
        QByteArray chunk = file->avatarData.mid(pos, length);
        nread = chunk.size();
        memcpy(data.get(), chunk.data(), nread);
    }
    else
    {
        file->file->seek(pos);
        nread = file->file->read((char*)data.get(), length);
        if (nread <= 0)
        {
            qWarning("onFileDataCallback: Failed to read from file");
            emit static_cast<Core*>(core)->fileTransferCancelled(*file);
            tox_file_send_chunk(tox, friendId, fileId, pos, nullptr, 0, nullptr);
            removeFile(friendId, fileId);
            return;
        }
        file->bytesSent += length;
    }

    if (!tox_file_send_chunk(tox, friendId, fileId, pos, data.get(), nread, nullptr))
    {
        qWarning("onFileDataCallback: Failed to send data chunk");
        return;
    }
    if (file->fileKind != TOX_FILE_KIND_AVATAR)
        emit static_cast<Core*>(core)->fileTransferInfo(*file);
}

void CoreFile::onFileRecvChunkCallback(Tox *tox, uint32_t friendId, uint32_t fileId, uint64_t position,
                                    const uint8_t *data, size_t length, void *core)
{
    //qDebug() << QString("Received chunk for %1:%2 pos %3 size %4")
    //                    .arg(friendId).arg(fileId).arg(position).arg(length);

    ToxFile* file = findFile(friendId, fileId);
    if (!file)
    {
        qWarning("onFileRecvChunkCallback: No such file in queue");
        tox_file_control(tox, friendId, fileId, TOX_FILE_CONTROL_CANCEL, nullptr);
        return;
    }

    if (file->bytesSent != position)
    {
        /// TODO: Allow ooo receiving for non-stream transfers, with very careful checking
        qWarning("onFileRecvChunkCallback: Received a chunk out-of-order, aborting transfer");
        if (file->fileKind != TOX_FILE_KIND_AVATAR)
            emit static_cast<Core*>(core)->fileTransferCancelled(*file);
        tox_file_control(tox, friendId, fileId, TOX_FILE_CONTROL_CANCEL, nullptr);
        removeFile(friendId, fileId);
        return;
    }

    if (!length)
    {
        if (file->fileKind == TOX_FILE_KIND_AVATAR)
        {
            QPixmap pic;
            pic.loadFromData(file->avatarData);
            if (!pic.isNull())
            {
                qDebug() << "Got"<<file->avatarData.size()<<"bytes of avatar data from"
                         << static_cast<Core*>(core)->getFriendUsername(friendId);
                Settings::getInstance().saveAvatar(pic, static_cast<Core*>(core)->getFriendAddress(friendId));
                Settings::getInstance().saveAvatarHash(file->fileName, static_cast<Core*>(core)->getFriendAddress(friendId));
                emit static_cast<Core*>(core)->friendAvatarChanged(friendId, pic);
            }
        }
        else
        {
            emit static_cast<Core*>(core)->fileTransferFinished(*file);
        }
        removeFile(friendId, fileId);
        return;
    }

    if (file->fileKind == TOX_FILE_KIND_AVATAR)
        file->avatarData.append((char*)data, length);
    else
        file->file->write((char*)data,length);
    file->bytesSent += length;

    if (file->fileKind != TOX_FILE_KIND_AVATAR)
        emit static_cast<Core*>(core)->fileTransferInfo(*file);
}

void CoreFile::onConnectionStatusChanged(Core* core, uint32_t friendId, bool online)
{
    /// TODO: Actually resume broken file transfers
    /// We need to:
    /// - Start a new file transfer with the same 32byte file ID with toxcore
    /// - Seek to the correct position again
    /// - Update the fileNum in our ToxFile
    /// - Update the users of our signals to check the 32byte tox file ID, not the uint32_t file_num (fileId)
    ToxFile::FileStatus status = online ? ToxFile::TRANSMITTING : ToxFile::BROKEN;
    for (uint64_t key : fileMap.keys())
    {
        if (key>>32 != friendId)
            continue;
        fileMap[key].status = status;
        emit core->fileTransferBrokenUnbroken(fileMap[key], !online);
    }
}
