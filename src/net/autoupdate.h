/*
    Copyright © 2014-2015 by The qTox Project

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


#ifndef AUTOUPDATE_H
#define AUTOUPDATE_H

#include <QString>
#include <QList>
#include <QMutex>
#include <sodium.h>
#include <atomic>
#include <functional>

#ifdef Q_OS_WIN
#define AUTOUPDATE_ENABLED 1
#elif defined(Q_OS_OSX)
#define AUTOUPDATE_ENABLED 1
#else
#define AUTOUPDATE_ENABLED 0
#endif

class AutoUpdater
{
public:
    struct UpdateFileMeta
    {
        unsigned char sig[crypto_sign_BYTES];
        QString id;
        QString installpath;
        uint64_t size;

        bool operator==(const UpdateFileMeta& other)
        {
            return (size == other.size
                    && id == other.id && installpath == other.installpath
                    && memcmp(sig, other.sig, crypto_sign_BYTES) == 0);
        }
    };

    struct UpdateFile
    {
        UpdateFileMeta metadata;
        QByteArray data;
    };

    struct VersionInfo
    {
        uint64_t timestamp;
        QString versionString;
    };

public:
    static void checkUpdatesAsyncInteractive();
    static bool isUpdateAvailable();
    static VersionInfo getUpdateVersion();
    static bool downloadUpdate();
    static bool isLocalUpdateReady();
    [[ noreturn ]] static void installLocalUpdate();
    static void abortUpdates();
    static QString getProgressVersion();
    static int getProgressValue();

protected:
    static QList<UpdateFileMeta> parseFlist(QByteArray flistData);
    static QByteArray getUpdateFlist();
    static QList<UpdateFileMeta> genUpdateDiff(QList<UpdateFileMeta> updateFlist);
    static bool isUpToDate(UpdateFileMeta file);
    static UpdateFile getUpdateFile(UpdateFileMeta fileMeta, std::function<void(int,int)> progressCallback);
    static void checkUpdatesAsyncInteractiveWorker();
    static void setProgressVersion(QString version);

private:
    AutoUpdater() = delete;

private:
    static const QString updateServer;
    static const QString platform;
    static const QString checkURI;
    static const QString flistURI;
    static const QString filesURI;
    static const QString updaterBin;
    static unsigned char key[];
    static std::atomic_bool abortFlag;
    static std::atomic_bool isDownloadingUpdate;
    static std::atomic<float> progressValue;
    static QString progressVersion;
    static QMutex progressVersionMutex;
};

#endif // AUTOUPDATE_H
