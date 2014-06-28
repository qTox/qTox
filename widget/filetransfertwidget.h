#ifndef FILETRANSFERTWIDGET_H
#define FILETRANSFERTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDateTime>

#include "core.h"

class ToxFile;

class FileTransfertWidget : public QWidget
{
    Q_OBJECT
public:
    FileTransfertWidget(ToxFile File);

public slots:
    void onFileTransferInfo(int FriendId, int FileNum, int Filesize, int BytesSent, ToxFile::FileDirection Direction);
    void onFileTransferCancelled(int FriendId, int FileNum, ToxFile::FileDirection Direction);
    void onFileTransferFinished(ToxFile File);

private slots:
    void cancelTransfer();
    void rejectRecvRequest();
    void acceptRecvRequest();
    void pauseResumeRecv();
    void pauseResumeSend();

private:
    QString getHumanReadableSize(int size);

private:
    QLabel *pic, *filename, *size, *speed, *eta;
    QPushButton *topright, *bottomright;
    QProgressBar *progress;
    QHBoxLayout *mainLayout, *textLayout;
    QVBoxLayout *infoLayout, *buttonLayout;
    QWidget* buttonWidget;
    QDateTime lastUpdate;
    long long lastBytesSent;
    int fileNum;
    int friendId;
    QString savePath;
    ToxFile::FileDirection direction;
    QString stopFileButtonStylesheet, pauseFileButtonStylesheet, acceptFileButtonStylesheet;
};

#endif // FILETRANSFERTWIDGET_H
