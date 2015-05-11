#include "profilelocker.h"
#include "src/misc/settings.h"
#include <QDir>
#include <QDebug>

using namespace std;

unique_ptr<QLockFile> ProfileLocker::lockfile;
QString ProfileLocker::curLockName;

QString ProfileLocker::lockPathFromName(const QString& name)
{
    return Settings::getInstance().getSettingsDirPath()+'/'+name+".lock";
}

bool ProfileLocker::isLockable(QString profile)
{
    // If we already have the lock, it's definitely lockable
    if (lockfile && curLockName == profile)
        return true;

    QLockFile newLock(lockPathFromName(profile));
    return newLock.tryLock();
}

bool ProfileLocker::lock(QString profile)
{
    if (lockfile && curLockName == profile)
        return true;

    QLockFile* newLock = new QLockFile(lockPathFromName(profile));
    if (!newLock->tryLock())
    {
        delete newLock;
        return false;
    }

    unlock();
    lockfile.reset(newLock);
    curLockName = profile;
    return true;
}

void ProfileLocker::unlock()
{
    if (!lockfile)
        return;
    lockfile->unlock();
    delete lockfile.release();
    lockfile = nullptr;
    curLockName.clear();
}

void ProfileLocker::clearAllLocks()
{
    qDebug() << "clearAllLocks: Wiping out all lock files";
    if (lockfile)
        unlock();

    QDir dir(Settings::getInstance().getSettingsDirPath());
    dir.setFilter(QDir::Files);
    dir.setNameFilters({"*.lock"});
    QFileInfoList files = dir.entryInfoList();
    for (QFileInfo fileInfo : files)
    {
        QFile file(fileInfo.absoluteFilePath());
        file.remove();
    }
}

void ProfileLocker::assertLock()
{
    if (!lockfile)
    {
        qCritical() << "assertLock: We don't seem to own any lock!";
        deathByBrokenLock();
    }

    if (!QFile(lockPathFromName(curLockName)).exists())
    {
        QString tmp = curLockName;
        unlock();
        if (lock(tmp))
        {
            qCritical() << "assertLock: Lock file was lost, but could be restored";
        }
        else
        {
            qCritical() << "assertLock: Lock file was lost, and could *NOT* be restored";
            deathByBrokenLock();
        }
    }
}

void ProfileLocker::deathByBrokenLock()
{
    qCritical() << "Lock is *BROKEN*, exiting immediately";
    abort();
}
