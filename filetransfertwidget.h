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
    FileTransfertWidget(ToxFile* File);

public slots:
    void onFileTransferInfo(ToxFile* File);
    void onFileTransferCancelled(ToxFile* File);
    void onFileTransferFinished(ToxFile* File);

private:
    QString getHumanReadableSize(int size);

private:
    QLabel *pic, *filename, *size, *speed, *eta;
    QPushButton *topright, *bottomright;
    QProgressBar *progress;
    ToxFile* file;
    QHBoxLayout *mainLayout, *textLayout;
    QVBoxLayout *infoLayout, *buttonLayout;
    QDateTime lastUpdate;
    long long lastBytesSent;
};

#endif // FILETRANSFERTWIDGET_H
