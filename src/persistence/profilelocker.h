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


#ifndef PROFILELOCKER_H
#define PROFILELOCKER_H

#include <QLockFile>
#include <memory>

/// Locks a Tox profile so that multiple instances can not use the same profile.
/// Only one lock can be acquired at the same time, which means
/// that there is little need for manually unlocking.
/// The current lock will expire if you exit or acquire a new one.
class ProfileLocker
{
private:
    ProfileLocker()=delete;

public:
    /// Checks if a profile is currently locked by *another* instance
    /// If we own the lock, we consider it lockable
    /// There is no guarantee that the result will still be valid by the
    /// time it is returned, this is provided on a best effort basis
    static bool isLockable(QString profile);
    /// Tries to acquire the lock on a profile, will not block
    /// Returns true if we already own the lock
    static bool lock(QString profile);
    /// Releases the lock on the current profile
    static void unlock();
    /// Returns true if we're currently holding a lock
    static bool hasLock();
    /// Return the name of the currently loaded profile, a null string if there is none
    static QString getCurLockName();
    /// Check that we actually own the lock
    /// In case the file was deleted on disk, restore it
    /// If we can't get a lock, exit qTox immediately
    /// If we never had a lock in the firt place, exit immediately
    static void assertLock();

private:
    static QString lockPathFromName(const QString& name);
    static void deathByBrokenLock(); ///< Print an error then exit immediately

private:
    static std::unique_ptr<QLockFile> lockfile;
    static QString curLockName;
};

#endif // PROFILELOCKER_H
