#ifndef PATHS_H
#define PATHS_H

#include <QString>
#include <QStringList>

class Paths
{
public:
    static Paths* makePaths(bool forcePortable = false, bool forceNonPortable = false);

    bool isPortable() const;
    QString getGlobalSettingsPath() const;
    QString getProfilesDir() const;
    QString getToxSaveDir() const;
    QString getAvatarsDir() const;
    QString getTransfersDir() const;
    QStringList getThemeDirs() const;
    QString getScreenshotsDir() const;

private:
    Paths();

private:
    QString basePath{};
    bool portable = false;
};

#endif // PATHS_H
