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

/// Handles checking and applying updates for qTox
class AutoUpdater
{
public:
    /// Connects to the qTox update server, returns true if an update is available for download
    /// Will call getUpdateVersion, and as such may block and processEvents
    static bool isUpdateAvailable();
    /// Fetch the version string of the last update available from the qTox update server
    /// Will try to follow qTox's proxy settings, may block and processEvents
    static QString getUpdateVersion();

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
