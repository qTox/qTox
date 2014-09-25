/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/
#ifndef FILETRANSFERINSTANCE_H
#define FILETRANSFERINSTANCE_H

#include <QObject>
#include <QDateTime>
#include <QImage>

#include "corestructs.h"

struct ToxFile;

class FileTransferInstance : public QObject
{
    Q_OBJECT
public:
    enum TransfState {tsPending, tsProcessing, tsPaused, tsFinished, tsCanceled};

public:
    explicit FileTransferInstance(ToxFile File);
    QString getHtmlImage();
    uint getId(){return id;}
    TransfState getState() {return state;}

public slots:
    void onFileTransferInfo(int FriendId, int FileNum, int64_t Filesize, int64_t BytesSent, ToxFile::FileDirection Direction);
    void onFileTransferCancelled(int FriendId, int FileNum, ToxFile::FileDirection Direction);
    void onFileTransferFinished(ToxFile File);
    void onFileTransferAccepted(ToxFile File);
    void onFileTransferPaused(int FriendId, int FileNum, ToxFile::FileDirection Direction);
    void onFileTransferRemotePausedUnpaused(ToxFile File, bool paused);
    void pressFromHtml(QString);

signals:
    void stateUpdated();

private slots:
    void cancelTransfer();
    void rejectRecvRequest();
    void acceptRecvRequest();
    void pauseResumeRecv();
    void pauseResumeSend();

private:
    QString getHumanReadableSize(unsigned long long size);
    QString QImage2base64(const QImage &img);
    QString drawButtonlessForm(const QString &type);
    QString draw2ButtonsForm(const QString &type, const QImage &imgA, const QImage &imgB);
    QString insertMiniature(const QString &type);
    QString wrapIntoForm(const QString &content, const QString &type, const QString &imgAstr, const QString &imgBstr);
    QImage drawProgressBarImg(const double &part, int w, int h);

private:
    static uint Idconter;
    uint id;

    TransfState state;
    bool remotePaused;
    QImage pic;
    QString filename, size, speed, eta;
    QDateTime lastUpdate;
    long long lastBytesSent, totalBytes;
    int fileNum;
    int friendId;
    QString savePath;
    ToxFile::FileDirection direction;
    QString stopFileButtonStylesheet, pauseFileButtonStylesheet, acceptFileButtonStylesheet;
};

#endif // FILETRANSFERINSTANCE_H
