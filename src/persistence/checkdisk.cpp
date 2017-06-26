
#include "checkdisk.h"
#include <QDebug>
#include <QSaveFile>
#include <QStorageInfo>
#include <QString>

// Consider less bytes available as full disk
#define RESERVE_SIZE 10485760

/**
 * @brief Check if disk is full.
 * @param path Path to file in filesystem.
 * @param ok Contains false if error occurs.
 * @return True if disk is full or error occured, else false.
 */
bool CheckDisk::diskFull(const QString& path, bool& noErr)
{
    QStorageInfo info(path);
    noErr = info.isValid();
    return info.bytesAvailable() < RESERVE_SIZE;
}

/**
 * @brief Checks disk usage.
 * @param path Path to file in filesystem.
 * @param ok Contains false if error occurs.
 * @return Return usage in percentage.
 */
int CheckDisk::diskUsage(const QString& path, bool& noErr)
{
    QStorageInfo info(path);
    noErr = info.isValid();
    return (100 - ((info.bytesAvailable() * 100) / info.bytesTotal()));
}

/**
 * @brief Checks if file can be written to disk.
 * @param path Path to any file/dir in filesystem.
 * @param file File to write.
 */
bool CheckDisk::canWrite(const QString& path, const QSaveFile& file)
{
    QStorageInfo info(path);
    return (info.bytesAvailable() != -1 && file.size() < info.bytesAvailable());
}
