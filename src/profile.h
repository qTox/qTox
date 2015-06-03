#ifndef PROFILE_H
#define PROFILE_H

#include <QVector>
#include <QString>
#include <QByteArray>

class Core;
class QThread;

/// Manages user profiles
class Profile
{
public:
    /// Locks and loads an existing profile and create the associate Core* instance
    /// Returns a nullptr on error, for example if the profile is already in use
    Profile* loadProfile(QString name, QString password);
    ~Profile();

    Core* getCore();
    void startCore(); ///< Starts the Core thread
    QByteArray loadToxSave(); ///< Loads the profile's .tox save from file, unencrypted

    /// Scan for profile, automatically importing them if needed
    /// NOT thread-safe
    static void scanProfiles();
    static QVector<QString> getProfiles();

    /// Checks whether a profile is encrypted. Return false on error.
    static bool isProfileEncrypted(QString name);

private:
    Profile(QString name, QString password);
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
    static QVector<QString> profiles;
    /// How much data we need to read to check if the file is encrypted
    /// Must be >= TOX_ENC_SAVE_MAGIC_LENGTH (8), which isn't publicly defined
    static constexpr int encryptHeaderSize = 8;
};

#endif // PROFILE_H
