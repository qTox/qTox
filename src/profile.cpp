#include "profile.h"
#include "src/misc/settings.h"
#include <cassert>
#include <QDir>
#include <QFileInfo>

QVector<QString> Profile::profiles;

Profile::Profile()
{

}

Profile::~Profile()
{

}

QVector<QString> Profile::getFilesByExt(QString extension)
{
    QDir dir(Settings::getInstance().getSettingsDirPath());
    QVector<QString> out;
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    dir.setNameFilters(QStringList("*."+extension));
    QFileInfoList list = dir.entryInfoList();
    out.reserve(list.size());
    for (QFileInfo file : list)
        out += file.completeBaseName();
    return out;
}

void Profile::scanProfiles()
{
    profiles.clear();
    QVector<QString> toxfiles = getFilesByExt("tox"), inifiles = getFilesByExt("ini");
    for (QString toxfile : toxfiles)
    {
        if (!inifiles.contains(toxfile))
            importProfile(toxfile);
        profiles.append(toxfile);
    }
}

void Profile::importProfile(QString name)
{
    Settings& s =  Settings::getInstance();
    assert(!s.profileExists(name));
    s.createPersonal(name);
}

QVector<QString> Profile::getProfiles()
{
    return profiles;
}
