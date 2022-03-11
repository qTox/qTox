/*
    Copyright © 2015-2019 by The qTox Project Contributors

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


#pragma once

#include <tox/tox.h>

#include "toxfile.h"
#include "src/core/core.h"
#include "src/core/toxpk.h"
#include "src/model/status.h"

#include "util/compatiblerecursivemutex.h"

#include <QHash>
#include <QMutex>
#include <QObject>
#include <QString>

#include <cstddef>
#include <cstdint>
#include <memory>

struct Tox;
class CoreFile;

using CoreFilePtr = std::unique_ptr<CoreFile>;

class CoreFile : public QObject
{
    Q_OBJECT

public:
    void handleAvatarOffer(uint32_t friendId, uint32_t fileId, bool accept, uint64_t filesize);
    static CoreFilePtr makeCoreFile(Core* core, Tox* tox, CompatibleRecursiveMutex& coreLoopLock);

    void sendFile(uint32_t friendId, QString filename, QString filePath,
                         long long filesize);
    void sendAvatarFile(uint32_t friendId, const QByteArray& data);
    void pauseResumeFile(uint32_t friendId, uint32_t fileId);
    void cancelFileSend(uint32_t friendId, uint32_t fileId);

    void cancelFileRecv(uint32_t friendId, uint32_t fileId);
    void rejectFileRecvRequest(uint32_t friendId, uint32_t fileId);
    void acceptFileRecvRequest(uint32_t friendId, uint32_t fileId, QString path);

    unsigned corefileIterationInterval();

signals:
    void fileSendStarted(ToxFile file);
    void fileReceiveRequested(ToxFile file);
    void fileTransferAccepted(ToxFile file);
    void fileTransferCancelled(ToxFile file);
    void fileTransferFinished(ToxFile file);
    void fileTransferPaused(ToxFile file);
    void fileTransferInfo(ToxFile file);
    void fileTransferRemotePausedUnpaused(ToxFile file, bool paused);
    void fileTransferBrokenUnbroken(ToxFile file, bool broken);
    void fileNameChanged(const ToxPk& friendPk);
    void fileSendFailed(uint32_t friendId, const QString& fname);

private:
    CoreFile(Tox* core_, CompatibleRecursiveMutex& coreLoopLock_);

    ToxFile* findFile(uint32_t friendId, uint32_t fileId);
    void addFile(uint32_t friendId, uint32_t fileId, const ToxFile& file);
    void removeFile(uint32_t friendId, uint32_t fileId);
    static constexpr uint64_t getFriendKey(uint32_t friendId, uint32_t fileId)
    {
        return (static_cast<std::uint64_t>(friendId) << 32) + fileId;
    }

    static void connectCallbacks(Tox& tox);
    static void onFileReceiveCallback(Tox* tox, uint32_t friendId, uint32_t fileId, uint32_t kind,
                                      uint64_t filesize, const uint8_t* fname, size_t fnameLen,
                                      void* vCore);
    static void onFileControlCallback(Tox* tox, uint32_t friendId, uint32_t fileId,
                                      Tox_File_Control control, void* vCore);
    static void onFileDataCallback(Tox* tox, uint32_t friendId, uint32_t fileId, uint64_t pos,
                                   size_t length, void* vCore);
    static void onFileRecvChunkCallback(Tox* tox, uint32_t friendId, uint32_t fileId, uint64_t position,
                                        const uint8_t* data, size_t length, void* vCore);

    static QString getCleanFileName(QString filename);

private slots:
    void onConnectionStatusChanged(uint32_t friendId, Status::Status state);

private:
    QHash<uint64_t, ToxFile> fileMap;
    Tox* tox;
    CompatibleRecursiveMutex* coreLoopLock = nullptr;
};
