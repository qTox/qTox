#ifndef CHECKDISK_H
#define CHECKDISK_H

class QString;
class QSaveFile;

// Warn user if disk usage is above threshold
#define DISK_WARN_THRESHOLD 95

namespace CheckDisk {

int diskUsage(const QString& path, bool& noErr);

bool diskFull(const QString& path, bool& noErr);

bool canWrite(const QString& path, const QSaveFile& file);
}

#endif // CHECKDISK_H
