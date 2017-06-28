
#include "checkdisk.h"
#include <QDebug>
#include <QMessageBox>
#include <QString>
#include <QWidget>
#include <stdint.h>


#ifdef Q_OS_LINUX
#include <errno.h>
#include <sys/vfs.h>
#endif

#ifdef Q_OS_FREEBSD
#include <errno.h>
#include <sys/mount.h>
#include <sys/param.h>
#endif

#ifdef Q_OS_OSX
#include <errno.h>
#include <sys/mount.h>
#include <sys/param.h>
#endif


/** Reserve size differ because of API provided by OS.
 * Windows returns bytes while POSIX systems use blocks.
 */
#ifdef Q_OS_WIN
#include <windows.h>
#define RESERVE_SIZE 102400
#else
#define RESERVE_SIZE 100
#endif


#define ERRC -1
#define OK 0


/**
 * @brief Fetches disk usage info. Windows uses bytes. Other OS blocks.
 * @param path Path to file in filesystem.
 * @param available Pointer to store available size.
 * @param total Pointer to store total size.
 * @param free Pointer to store free size.
 * @return OK on succes, ERRC on error.
 */
static int diskInfo(const QString& path, uint64_t* available, uint64_t* total, uint64_t* free)
{
#ifdef Q_OS_WIN
    char_t* c_path = path.toStdString().c_str();
    bool ret = GetDiskFreeSpaceEx(c_path, (PULARGE_INTEGER)available, (PULARGE_INTEGER)total,
                                  (PULARGE_INTEGER)free);
    if (!ret) {
        qCritical() << "Error reading disk size at :" << path << ":" << GetLastError();
        return ERRC;
    }
#else
    struct statfs info;
    if (statfs(path.toStdString().c_str(), &info) != 0) {
        qCritical() << "Error reading disk size at :" << path << ":" << strerror(errno);
        return ERRC;
    }
    *available = info.f_bavail;
    *total = info.f_blocks;
    *free = info.f_bfree;
#endif
    return OK;

}

/**
 * @brief Check is disk full.
 * @param path Path to file in filesystem.
 * @param ok Stores success status.
 * @return True if disk is full or error occured, else false.
 */
bool CheckDisk::diskFull(const QString& path)
{
    uint64_t available, total, free;
    if (diskInfo(path, &available, &total, &free) == OK) {
        return available < RESERVE_SIZE;
    }
    return true;
}

/**
 * @brief Checks disk usage.
 * @param path Path to file in filesystem.
 * @return Return usage in percentage. On error returns -1.
 */
int CheckDisk::diskUsage(const QString& path)
{
    uint64_t available, total, free;
    if (diskInfo(path, &available, &total, &free) == OK) {
        return (100 - ((available * 100) / total));
    }
    return -1;
}

/**
 * @brief Shows error message for full disk.
 * @param parent Parent widget
 */
void CheckDisk::errorDialog(QWidget* parent)
{
    qCritical() << "Disk full";
}
