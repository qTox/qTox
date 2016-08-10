/*
    Copyright © 2015 by The qTox Project

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


#ifndef COREFILE_H
#define COREFILE_H

#include <cstdint>
#include <cstddef>
#include <memory>
#include <tox/tox.h>

#include "corestructs.h"

#include <QString>
#include <QMutex>
#include <QHash>

struct Tox;
class Core;

class CoreFile
{
    friend class Core;

private:
    CoreFile()=delete;

    // Internal file sending APIs, used by Core. Public API in core.h
private:
    static void sendFile(Core *core, uint32_t friendId, QString filename, QString filePath, long long filesize);
    static void sendAvatarFile(Core* core, uint32_t friendId, const QByteArray& data);
    static void pauseResumeFileSend(Core* core, uint32_t friendId, uint32_t fileId);
    static void pauseResumeFileRecv(Core* core, uint32_t friendId, uint32_t fileId);
    static void cancelFileSend(Core* core, uint32_t friendId, uint32_t fileId);
    static void cancelFileRecv(Core* core, uint32_t friendId, uint32_t fileId);
    static void rejectFileRecvRequest(Core* core, uint32_t friendId, uint32_t fileId);
    static void acceptFileRecvRequest(Core* core, uint32_t friendId, uint32_t fileId, QString path);
    static ToxFile *findFile(uint32_t friendId, uint32_t fileId);
    static void addFile(uint32_t friendId, uint32_t fileId, const ToxFile& file);
    static void removeFile(uint32_t friendId, uint32_t fileId);
    static unsigned corefileIterationInterval();

private:
    static void onFileReceiveCallback(Tox*, uint32_t friendnumber, uint32_t fileId, uint32_t kind,
                                      uint64_t filesize, const uint8_t *fname, size_t fnameLen, void *core);
    static void onFileControlCallback(Tox *tox, uint32_t friendId, uint32_t fileId,
                                      TOX_FILE_CONTROL control, void *core);
    static void onFileDataCallback(Tox *tox, uint32_t friendId, uint32_t fileId,
                                   uint64_t pos, size_t length, void *core);
    static void onFileRecvChunkCallback(Tox *tox, uint32_t friendId, uint32_t fileId, uint64_t position,
                                        const uint8_t *data, size_t length, void *core);
    static void onConnectionStatusChanged(Core* core, uint32_t friendId, bool online);

private:
    static QMutex fileSendMutex;
    static QHash<uint64_t, ToxFile> fileMap;
};

#endif // COREFILE_H
