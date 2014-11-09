/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

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

/// Handles checking and applying updates for qTox
class AutoUpdater
{
public:
    struct UpdateFileMeta
    {
        unsigned char sig[crypto_sign_BYTES]; ///< Signature of the file (ed25519)
        QString id; ///< Unique id of the file
        QString installpath; ///< Local path including the file name. May be relative to qtox-updater or absolute
        uint64_t size; ///< Size in bytes of the file
    };

    struct UpdateFile
    {
        UpdateFileMeta metadata;
        QByteArray data;
    };

public:
    /// Connects to the qTox update server, returns true if an update is available for download
    /// Will call getUpdateVersion, and as such may block and processEvents
    static bool isUpdateAvailable();
    /// Fetch the version string of the last update available from the qTox update server
    /// Will try to follow qTox's proxy settings, may block and processEvents
    static QString getUpdateVersion();
    /// Generates a list of files we need to update
    /// Will try to follow qTox's proxy settings, may block and processEvents
    static QList<UpdateFileMeta> genUpdateDiff();

protected:
    /// Parses and validates a flist file. Returns an empty list on error
    static QList<UpdateFileMeta> parseflist(QByteArray flistData);
    /// Get the update server's flist and parse it. Returns an empty list on error
    /// Will try to follow qTox's proxy settings, may block and processEvents
    static QList<UpdateFileMeta> getUpdateFlist();

private:
    AutoUpdater() = delete;

private:
    // Constants
    static const QString updateServer; ///< Hostname of the qTox update server
    static const QString platform; ///< Name of platform we're trying to get updates for
    static const QString checkURI; ///< URI of the file containing the latest version string
    static const QString flistURI; ///< URI of the file containing info on each file (hash, signature, size, name, ..)
    static const QString filesURI; ///< URI of the actual files of the latest version
    static unsigned char key[];
};

#endif // AUTOUPDATE_H
