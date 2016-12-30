/*
    Copyright © 2015-2016 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "core.h"
#include "corefile.h"
#include "corestructs.h"
#include "src/core/cstring.h"
#include "src/persistence/settings.h"
#include "src/persistence/profile.h"
#include <QDebug>
#include <QFile>
#include <QThread>
#include <QDir>
#include <memory>

/**
 * @class CoreFile
 * @brief Implements Core's file transfer callbacks.
 *
 * Avoids polluting core.h with private internal callbacks.
 */

QMutex CoreFile::fileSendMutex;
QHash<uint64_t, ToxFile> CoreFile::fileMap;
using namespace std;

/**
 * @brief Get corefile iteration interval.
 *
 * tox_iterate calls to get good file transfer performances
 * @return The maximum amount of time in ms that Core should wait between two tox_iterate() calls.
 */
unsigned CoreFile::corefileIterationInterval()
{
    /*
       Sleep at most 1000ms if we have no FT, 10 for user FTs
       There is no real difference between 10ms sleep and 50ms sleep when it
       comes to CPU usage – just keep the CPU usage low when there are no file
       transfers, and speed things up when there is an ongoing file transfer.
    */
    constexpr unsigned fileInterval =   10,
                       idleInterval = 1000;

    for (ToxFile& file : fileMap)
    {
        if (file.status == ToxFile::TRANSMITTING)
        {
            return fileInterval;
        }
    }
    return idleInterval;
}

void CoreFile::sendAvatarFile(Core* core, uint32_t friendId, const QByteArray& data)
{
    QMutexLocker mlocker(&fileSendMutex);

    if (data.isEmpty())
    {
        tox_file_send(core->tox, friendId, TOX_FILE_KIND_AVATAR, 0,
                                         nullptr, nullptr, 0, nullptr);
        return;
    }

    static_assert(TOX_HASH_LENGTH <= TOX_FILE_ID_LENGTH, "TOX_HASH_LENGTH > TOX_FILE_ID_LENGTH!");
    uint8_t avatarHash[TOX_HASH_LENGTH];
    tox_hash(avatarHash, (uint8_t*)data.data(), data.size());
    uint64_t filesize = data.size();

    TOX_ERR_FILE_SEND error;
    uint32_t fileNum = tox_file_send(core->tox, friendId, TOX_FILE_KIND_AVATAR, filesize,
                                         avatarHash, avatarHash, TOX_HASH_LENGTH, &error);

    switch (error)
    {
    case TOX_ERR_FILE_SEND_OK:
        break;
    case TOX_ERR_FILE_SEND_FRIEND_NOT_CONNECTED:
        qCritical() << "Friend not connected";
        return;
    case TOX_ERR_FILE_SEND_FRIEND_NOT_FOUND:
        qCritical() << "Friend not found";
        return;
    case TOX_ERR_FILE_SEND_NAME_TOO_LONG:
        qCritical() << "Name too long";
        return;
    case TOX_ERR_FILE_SEND_NULL:
        qCritical() << "Send null";
        return;
    case TOX_ERR_FILE_SEND_TOO_MANY:
        qCritical() << "To many ougoing transfer";
        return;
    default:
        return;
    }

    ToxFile file{fileNum, friendId, "", "", ToxFile::SENDING};
    file.filesize = filesize;
    file.fileName = QByteArray((char*)avatarHash, TOX_HASH_LENGTH);
    file.fileKind = TOX_FILE_KIND_AVATAR;
    file.avatarData = data;
    file.resumeFileId.resize(TOX_FILE_ID_LENGTH);
    tox_file_get_file_id(core->tox, friendId, fileNum, (uint8_t*)file.resumeFileId.data(), nullptr);
    addFile(friendId, fileNum, file);
}

void CoreFile::sendFile(Core* core, uint32_t friendId, QString filename, QString filePath, long long filesize)
{
    QMutexLocker mlocker(&fileSendMutex);

    QByteArray fileName = filename.toUtf8();
    uint32_t fileNum = tox_file_send(core->tox, friendId, TOX_FILE_KIND_DATA, filesize, nullptr,
                                (uint8_t*)fileName.data(), fileName.size(), nullptr);
    if (fileNum == std::numeric_limits<uint32_t>::max())
    {
        qWarning() << "sendFile: Can't create the Tox file sender";
        emit core->fileSendFailed(friendId, filename);
        return;
    }
    qDebug() << QString("sendFile: Created file sender %1 with friend %2").arg(fileNum).arg(friendId);

    ToxFile file{fileNum, friendId, fileName, filePath, ToxFile::SENDING};
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
    {
        qWarning() << "pauseResumeFileSend: File is stopped";
    }
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
    {
        qWarning() << "pauseResumeFileRecv: File is stopped or broken";
    }
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
    uint64_t key = getFriendKey(friendId, fileId);
    if (fileMap.contains(key))
    {
        return &fileMap[key];
    }

    qWarning() << "findFile: File transfer with ID" << friendId << ':'
               << fileId << "doesn't exist";
    return nullptr;
}

void CoreFile::addFile(uint32_t friendId, uint32_t fileId, const ToxFile& file)
{
    uint64_t key = getFriendKey(friendId, fileId);

    if (fileMap.contains(key))
    {
        qWarning() << "addFile: Overwriting existing file transfer with same ID"
                   << friendId << ':' << fileId;
    }

    fileMap.insert(key, file);
}

void CoreFile::removeFile(uint32_t friendId, uint32_t fileId)
{
    uint64_t key = getFriendKey(friendId, fileId);
    if (!fileMap.contains(key))
    {
        qWarning() << "removeFile: No such file in queue";
        return;
    }
    fileMap[key].file->close();
    fileMap.remove(key);
}

void CoreFile::onFileReceiveCallback(Tox*, uint32_t friendId, uint32_t fileId,
                                     uint32_t kind, uint64_t filesize,
                                     const uint8_t* fname, size_t fnameLen,
                                     void* vCore)
{
    Core* core = static_cast<Core*>(vCore);

    if (kind == TOX_FILE_KIND_AVATAR)
    {
        QString friendAddr = core->getFriendPublicKey(friendId);
        if (!filesize)
        {
            qDebug() << QString("Received empty avatar request %1:%2").arg(friendId).arg(fileId);
            // Avatars of size 0 means explicitely no avatar
            emit core->friendAvatarRemoved(friendId);
            core->profile.removeAvatar(friendAddr);
            return;
        }
        else
        {
            static_assert(TOX_HASH_LENGTH <= TOX_FILE_ID_LENGTH, "TOX_HASH_LENGTH > TOX_FILE_ID_LENGTH!");
            uint8_t avatarHash[TOX_FILE_ID_LENGTH];
            tox_file_get_file_id(core->tox, friendId, fileId, avatarHash, nullptr);
            if (core->profile.getAvatarHash(friendAddr) == QByteArray((char*)avatarHash, TOX_HASH_LENGTH))
            {
                // If it's an avatar but we already have it cached, cancel
                qDebug() << QString("Received avatar request %1:%2, reject, since we have it in cache.").arg(friendId).arg(fileId);
                tox_file_control(core->tox, friendId, fileId, TOX_FILE_CONTROL_CANCEL, nullptr);
                return;
            }
            else
            {
                // It's an avatar and we don't have it, autoaccept the transfer
                qDebug() << QString("Received avatar request %1:%2, accept, since we don't have it in cache.").arg(friendId).arg(fileId);
                tox_file_control(core->tox, friendId, fileId, TOX_FILE_CONTROL_RESUME, nullptr);
            }
        }
    }
    else
    {
        qDebug() << QString("Received file request %1:%2 kind %3")
                            .arg(friendId).arg(fileId).arg(kind);
    }

    ToxFile file{fileId, friendId, QByteArray((char*)fname,fnameLen), "", ToxFile::RECEIVING};
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
        if (file->fileKind != TOX_FILE_KIND_AVATAR)
            qDebug() << "File tranfer"<<friendId<<":"<<fileId<<"cancelled by friend";
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
        if (file->direction == ToxFile::SENDING && file->fileKind == TOX_FILE_KIND_AVATAR)
            qDebug() << "Avatar transfer"<<fileId<<"to friend"<<friendId<<"accepted";
        else
            qDebug() << "onFileControlCallback: Received resume for file "<<friendId<<":"<<fileId;
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

    ToxFile* file = findFile(friendId, fileId);
    if (!file)
    {
        qWarning("onFileDataCallback: No such file in queue");
        return;
    }

    // If we reached EOF, ack and cleanup the transfer
    if (!length)
    {
        if (file->fileKind != TOX_FILE_KIND_AVATAR)
        {
            emit static_cast<Core*>(core)->fileTransferFinished(*file);
            emit static_cast<Core*>(core)->fileUploadFinished(file->filePath);
        }
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

void CoreFile::onFileRecvChunkCallback(Tox *tox, uint32_t friendId,
                                       uint32_t fileId, uint64_t position,
                                       const uint8_t* data, size_t length,
                                       void* vCore)
{
    Core* core = static_cast<Core*>(vCore);
    ToxFile* file = findFile(friendId, fileId);
    if (!file)
    {
        qWarning("onFileRecvChunkCallback: No such file in queue");
        tox_file_control(tox, friendId, fileId, TOX_FILE_CONTROL_CANCEL, nullptr);
        return;
    }

    if (file->bytesSent != position)
    {
        qWarning("onFileRecvChunkCallback: Received a chunk out-of-order, aborting transfer");
        if (file->fileKind != TOX_FILE_KIND_AVATAR)
            emit core->fileTransferCancelled(*file);
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
                qDebug() << "Got"<<file->avatarData.size()<<"bytes of avatar data from" <<friendId;
                core->profile.saveAvatar(file->avatarData, core->getFriendPublicKey(friendId));
                emit core->friendAvatarChanged(friendId, pic);
            }
        }
        else
        {
            emit core->fileTransferFinished(*file);
            emit core->fileDownloadFinished(file->filePath);
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
    // TODO: Actually resume broken file transfers
    // We need to:
    // - Start a new file transfer with the same 32byte file ID with toxcore
    // - Seek to the correct position again
    // - Update the fileNum in our ToxFile
    // - Update the users of our signals to check the 32byte tox file ID, not the uint32_t file_num (fileId)
    ToxFile::FileStatus status = online ? ToxFile::TRANSMITTING : ToxFile::BROKEN;
    for (uint64_t key : fileMap.keys())
    {
        if (key>>32 != friendId)
            continue;
        fileMap[key].status = status;
        emit core->fileTransferBrokenUnbroken(fileMap[key], !online);
    }
}
