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

/// Implements Core's file transfer callbacks
/// Avoids polluting core.h with private internal callbacks
class CoreFile
{
    friend class Core;

private:
    CoreFile()=delete;

    // Internal file sending APIs, used by Core. Public API in core.h
private:
    static void sendFile(Core *core, uint32_t friendId, QString Filename, QString FilePath, long long filesize);
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
    /// Returns the maximum amount of time in ms that Core should wait between two
    /// tox_iterate calls to get good file transfer performances
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
