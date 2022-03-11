/*
    Copyright © 2019 by The qTox Project Contributors

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

#include "src/core/toxfilepause.h"
#include "src/core/toxfileprogress.h"

#include <QString>
#include <memory>
#include <QCryptographicHash>

class QFile;
class QTimer;

struct ToxFile
{
    // Note do not change values, these are directly inserted into the DB in their
    // current form, changing order would mess up database state!
    enum FileStatus
    {
        INITIALIZING = 0,
        PAUSED = 1,
        TRANSMITTING = 2,
        BROKEN = 3,
        CANCELED = 4,
        FINISHED = 5,
    };

    // Note do not change values, these are directly inserted into the DB in their
    // current form (can add fields though as db representation is an int)
    enum FileDirection : bool
    {
        SENDING = 0,
        RECEIVING = 1,
    };

    ToxFile();
    ToxFile(uint32_t fileNum_, uint32_t friendId_, QString fileName_, QString filePath_,
            uint64_t filesize, FileDirection direction);

    bool operator==(const ToxFile& other) const;
    bool operator!=(const ToxFile& other) const;

    void setFilePath(QString path);
    bool open(bool write);

    uint8_t fileKind;
    uint32_t fileNum;
    uint32_t friendId;
    QString fileName;
    QString filePath;
    std::shared_ptr<QFile> file;
    FileStatus status;
    FileDirection direction;
    QByteArray avatarData;
    QByteArray resumeFileId;
    std::shared_ptr<QCryptographicHash> hashGenerator = std::make_shared<QCryptographicHash>(QCryptographicHash::Sha256);
    ToxFilePause pauseStatus;
    ToxFileProgress progress;
};
