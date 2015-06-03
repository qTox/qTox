#ifndef PROFILE_H
#define PROFILE_H

#include <QVector>
#include <QString>

/// Manages user profiles
class Profile
{
public:
    Profile();
    ~Profile();

    /// Scan for profile, automatically importing them if needed
    /// NOT thread-safe
    static void scanProfiles();
    static QVector<QString> getProfiles();

private:
    /// Lists all the files in the config dir with a given extension
    /// Pass the raw extension, e.g. "jpeg" not ".jpeg".
    static QVector<QString> getFilesByExt(QString extension);
    /// Creates a .ini file for the given .tox profile
    /// Only pass the basename, without extension
    static void importProfile(QString name);

private:
    static QVector<QString> profiles;
};

#endif // PROFILE_H
