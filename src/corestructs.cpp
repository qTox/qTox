#include "corestructs.h"
#include <QFile>

ToxFile::ToxFile(int FileNum, int FriendId, QByteArray FileName, QString FilePath, FileDirection Direction)
    : fileNum(FileNum), friendId(FriendId), fileName{FileName}, filePath{FilePath}, file{new QFile(filePath)},
    bytesSent{0}, filesize{0}, status{STOPPED}, direction{Direction}, sendTimer{nullptr}
{
}

void ToxFile::setFilePath(QString path)
{
    filePath=path;
    file->setFileName(path);
}

bool ToxFile::open(bool write)
{
    return write ? file->open(QIODevice::ReadWrite) : file->open(QIODevice::ReadOnly);
}
