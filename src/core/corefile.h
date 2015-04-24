#ifndef COREFILE_H
#define COREFILE_H

#include <cstdint>
#include <cstddef>
#include <tox/tox.h>

struct Tox;

/// Implements Core's file transfer callbacks
/// Avoids polluting core.h with private internal callbacks
class CoreFile
{
private:
    CoreFile()=delete;

public:
    static void onFileReceiveCallback(Tox*, uint32_t friendnumber, uint32_t fileId, uint32_t kind,
                                      uint64_t filesize, const uint8_t *fname, size_t fnameLen, void *core);
    static void onFileControlCallback(Tox *tox, uint32_t friendnumber, uint32_t filenumber,
                                      TOX_FILE_CONTROL control, void *core);
    static void onFileDataCallback(Tox *tox, uint32_t friendnumber, uint32_t filenumber,
                                   uint64_t pos, size_t length, void *core);
    static void onFileRecvChunkCallback(Tox *tox, uint32_t friendId, uint32_t fileId, uint64_t position,
                                        const uint8_t *data, size_t length, void *core);
};

#endif // COREFILE_H
