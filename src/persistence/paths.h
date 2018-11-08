#ifndef PATHS_H
#define PATHS_H

#include <QString>
#include <QStringList>

class Paths
{
public:
    enum class Portable {
        Auto,           /** Auto detect if portable or non-portable */
        Portable,       /** Force portable mode */
        NonPortable     /** Force non-portable mode */
    };

    static Paths* makePaths(Portable mode = Portable::Auto);

    bool isPortable() const;
    QString getGlobalSettingsPath() const;
    QString getProfilesDir() const;
    QString getToxSaveDir() const;
    QString getAvatarsDir() const;
    QString getTransfersDir() const;
    QStringList getThemeDirs() const;
    QString getScreenshotsDir() const;

private:
    Paths(const QString &basePath, bool portable);

private:
    QString basePath{};
    bool portable = false;
};

#endif // PATHS_H
