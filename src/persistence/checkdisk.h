#ifndef CHECKDISK_H
#define CHECKDISK_H

#include <cstdint>

#include <QMessageBox>
#include <QString>
#include <QWidget>

// Warn user if disk usage is above threshold
#define DISK_WARN_THRESHOLD 95

namespace DiskCheck {

int diskUsage(const QString& path);

bool diskFull(const QString& path);

void errorDialog(QWidget* parent = 0);
}


#endif // CHECKDISK_H
