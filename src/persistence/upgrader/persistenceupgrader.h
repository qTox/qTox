#ifndef PERSISTENCEUPGRADER_H
#define PERSISTENCEUPGRADER_H

#include <QString>

class PersistenceUpgrader
{
public:
    static bool runUpgrade(bool portable);

private:
    PersistenceUpgrader(bool portable);
    bool isUpgradeNeeded();
    bool doUpgrade();

    bool isVersion1_16_3();
    QString getSettingsPath_1_17_0();
    QString versionFromFile();
    QString getSettingsDir_1_16_3() const;

    bool checkedMove(QString source, QString dest);

    QString getProfilesDir_1_17_0() const;
    QString getGlobalSettingsPath_1_17_0() const;
    bool upgradeFrom_1_16_3();

private:
    bool portable;
    QString curVersion{};
};

#endif // PERSISTENCEUPGRADER_H
