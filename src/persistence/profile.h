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


#ifndef PROFILE_H
#define PROFILE_H

#include <QVector>
#include <QString>
#include <QByteArray>
#include <tox/toxencryptsave.h>

class Core;
class QThread;

/// Manages user profiles
class Profile
{
public:
    /// Locks and loads an existing profile and create the associate Core* instance
    /// Returns a nullptr on error, for example if the profile is already in use
    static Profile* loadProfile(QString name, QString password = QString());
    /// Creates a new profile and the associated Core* instance
    /// If password is not empty, the profile will be encrypted
    /// Returns a nullptr on error, for example if the profile already exists
    static Profile* createProfile(QString name, QString password);
    ~Profile();

    Core* getCore();
    QString getName();

    void startCore(); ///< Starts the Core thread
    void restartCore(); ///< Delete core and restart a new one
    bool isNewProfile();
    bool isEncrypted(); ///< Returns true if we have a password set (doesn't check the actual file on disk)
    bool checkPassword(); ///< Checks whether the password is valid
    QString getPassword();
    void setPassword(QString newPassword); ///< Changes the encryption password and re-saves everything with it
    const TOX_PASS_KEY& getPasskey();

    QByteArray loadToxSave(); ///< Loads the profile's .tox save from file, unencrypted
    void saveToxSave(); ///< Saves the profile's .tox save, encrypted if needed. Invalid on deleted profiles.
    void saveToxSave(QByteArray data); ///< Write the .tox save, encrypted if needed. Invalid on deleted profiles.

    /// Removes the profile permanently
    /// It is invalid to call loadToxSave or saveToxSave on a deleted profile
    /// Updates the profiles vector
    void remove();

    /// Tries to rename the profile
    bool rename(QString newName);

    /// Scan for profile, automatically importing them if needed
    /// NOT thread-safe
    static void scanProfiles();
    static QVector<QString> getProfiles();

    static bool exists(QString name);
    static bool isEncrypted(QString name); ///< Returns false on error. Checks the actual file on disk.

private:
    Profile(QString name, QString password, bool newProfile);
    /// Lists all the files in the config dir with a given extension
    /// Pass the raw extension, e.g. "jpeg" not ".jpeg".
    static QVector<QString> getFilesByExt(QString extension);
    /// Creates a .ini file for the given .tox profile
    /// Only pass the basename, without extension
    static void importProfile(QString name);

private:
    Core* core;
    QThread* coreThread;
    QString name, password;
    TOX_PASS_KEY passkey;
    bool newProfile; ///< True if this is a newly created profile, with no .tox save file yet.
    bool isRemoved; ///< True if the profile has been removed by remove()
    static QVector<QString> profiles;
    /// How much data we need to read to check if the file is encrypted
    /// Must be >= TOX_ENC_SAVE_MAGIC_LENGTH (8), which isn't publicly defined
    static constexpr int encryptHeaderSize = 8;
};

#endif // PROFILE_H
