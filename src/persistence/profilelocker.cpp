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


#include "profilelocker.h"
#include "src/persistence/settings.h"
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
    newLock->setStaleLockTime(0);
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

bool ProfileLocker::hasLock()
{
    return lockfile.operator bool();
}

QString ProfileLocker::getCurLockName()
{
    if (lockfile)
        return curLockName;
    else
        return QString();
}
