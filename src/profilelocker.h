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
    /// Releases all locks on all profiles
    /// DO NOT call unless all we're the only qTox instance
    /// and we don't hold any lock yet.
    static void clearAllLocks();
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
