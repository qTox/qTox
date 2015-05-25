/*
    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/


#ifndef AUTOUPDATE_H
#define AUTOUPDATE_H

#include <QString>
#include <QList>
#include <sodium.h>
#include <atomic>

/// For now we only support auto updates on Windows and OS X, although extending it is not a technical issue.
/// Linux users are expected to use their package managers or update manually through official channels.
#ifdef Q_OS_WIN
#define AUTOUPDATE_ENABLED 1
#elif defined(Q_OS_OSX)
#define AUTOUPDATE_ENABLED 1
#else
#define AUTOUPDATE_ENABLED 0
#endif

/// Handles checking and applying updates for qTox
/// Do *NOT* use auto update unless AUTOUPDATE_ENABLED is defined to 1
class AutoUpdater
{
public:
    struct UpdateFileMeta
    {
        unsigned char sig[crypto_sign_BYTES]; ///< Signature of the file (ed25519)
        QString id; ///< Unique id of the file
        QString installpath; ///< Local path including the file name. May be relative to qtox-updater or absolute
        uint64_t size; ///< Size in bytes of the file

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
    /// Connects to the qTox update server, if an updat is found shows a dialog to the user asking to download it
    /// Runs asynchronously in its own thread, and will return immediatly
    /// Will call isUpdateAvailable, and as such may processEvents
    static void checkUpdatesAsyncInteractive();
    /// Connects to the qTox update server, returns true if an update is available for download
    /// Will call getUpdateVersion, and as such may block and processEvents
    static bool isUpdateAvailable();
    /// Fetch the version info of the last update available from the qTox update server
    /// Will try to follow qTox's proxy settings, may block and processEvents
    static VersionInfo getUpdateVersion();
    /// Will try to download an update, if successful returns true and qTox will apply it after a restart
    /// Will try to follow qTox's proxy settings, may block and processEvents
    static bool downloadUpdate();
    /// Returns true if an update is downloaded and ready to be installed,
    /// if so, call installLocalUpdate.
    /// If an update was partially downloaded, the function will resume asynchronously and return false
    static bool isLocalUpdateReady();
    /// Launches the qTox updater to try to install the local update and exits immediately
    /// Will not check that the update actually exists, use isLocalUpdateReady first for that
    /// The qTox updater will restart us after the update is done
    /// Note: If we fail to start the qTox updater, we will delete the update and exit
    [[ noreturn ]] static void installLocalUpdate();
    /// Aborting will make some functions try to return early
    /// Call before qTox exits to avoid the updater running in the background
    static void abortUpdates();

protected:
    /// Parses and validates a flist file. Returns an empty list on error
    static QList<UpdateFileMeta> parseFlist(QByteArray flistData);
    /// Gets the update server's flist. Returns an empty array on error
    /// Will try to follow qTox's proxy settings, may block and processEvents
    static QByteArray getUpdateFlist();
    /// Gets the local flist. Returns an empty array on error
    static QByteArray getLocalFlist();
    /// Generates a list of files we need to update
    static QList<UpdateFileMeta> genUpdateDiff(QList<UpdateFileMeta> updateFlist);
    /// Tries to fetch the file from the update server. Returns a file with a null QByteArray on error.
    /// Note that a file with an empty but non-null QByteArray is not an error, merely a file of size 0.
    /// Will try to follow qTox's proxy settings, may block and processEvents
    static UpdateFile getUpdateFile(UpdateFileMeta fileMeta);
    /// Does the actual work for checkUpdatesAsyncInteractive
    /// Blocking, but otherwise has the same properties than checkUpdatesAsyncInteractive
    static void checkUpdatesAsyncInteractiveWorker();

private:
    AutoUpdater() = delete;

private:
    // Constants
    static const QString updateServer; ///< Hostname of the qTox update server
    static const QString platform; ///< Name of platform we're trying to get updates for
    static const QString checkURI; ///< URI of the file containing the latest version string
    static const QString flistURI; ///< URI of the file containing info on each file (hash, signature, size, name, ..)
    static const QString filesURI; ///< URI of the actual files of the latest version
    static const QString updaterBin; ///< Path to the qtox-updater binary
    static unsigned char key[];
    static bool abortFlag; ///< If true, try to abort everything.
    static std::atomic_bool isDownloadingUpdate; ///< We'll pretend there's no new update available if we're already updating
};

#endif // AUTOUPDATE_H
