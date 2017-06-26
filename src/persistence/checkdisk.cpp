
#include "checkdisk.h"
#include <QDebug>
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

static int diskInfo(const QString& path, uint64_t* available, uint64_t* total, uint64_t* free)
{
#ifdef Q_OS_WIN
    char_t* c_path = path.toStdString().c_str();
    bool ret = GetDiskFreeSpaceEx(c_path, (PULARGE_INTEGER)available, (PULARGE_INTEGER)total,
                                  (PULARGE_INTEGER)free);
    if (!ret) {
        qDebug() << "Error reading disk size at :" << path << ":" << GetLastError();
        return ERRC;
    }
    return OK;
#else
    struct statfs info;
    if (statfs(path.toStdString().c_str(), &info) != 0) {
        qWarning() << "Error reading disk size at :" << path << ":" << strerror(errno);
        return ERRC;
    }
    *available = info.f_bavail;
    *total = info.f_blocks;
    *free = info.f_bfree;
    return OK;
#endif
}

/**
 * @brief Checks disk usage.
 * @param path Path to file in filesystem.
 * @return True if disk is full. Anything else including error is false.
 */
bool DiskCheck::diskFull(const QString& path)
{
    uint64_t available, total, free;
    if (diskInfo(path, &available, &total, &free) == OK) {
        return available < RESERVE_SIZE;
    }
    return false;
}

/**
 * @brief Checks disk usage.
 * @param path Path to file in filesystem.
 * @return Return usage in percentage. On error returns -1.
 */
int DiskCheck::diskUsage(const QString& path)
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
void DiskCheck::errorDialog(QWidget* parent)
{
    qCritical() << "Disk full";
    QMessageBox error(parent);
    error.critical(0, "Error", "Disk full!");
    error.setFixedSize(500, 200);
}
