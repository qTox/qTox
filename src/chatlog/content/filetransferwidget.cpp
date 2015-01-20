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

#include "filetransferwidget.h"
#include "ui_filetransferwidget.h"

#include "src/core.h"
#include "src/misc/style.h"

#include <QMouseEvent>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QDesktopServices>
#include <QDebug>

FileTransferWidget::FileTransferWidget(QWidget *parent, ToxFile file)
    : QWidget(parent)
    , ui(new Ui::FileTransferWidget)
    , fileInfo(file)
    , lastTick(QTime::currentTime())
{
    ui->setupUi(this);

    // hide the QWidget background (background-color: transparent doesn't seem to work)
    setAttribute(Qt::WA_TranslucentBackground, true);

    ui->previewLabel->hide();
    ui->filenameLabel->setText(file.fileName);
    ui->progressBar->setValue(0);
    ui->fileSizeLabel->setText(getHumanReadableSize(file.filesize));
    ui->progressLabel->setText("0kiB/s");
    ui->etaLabel->setText("");

    setStyleSheet(Style::getStylesheet(":/ui/fileTransferInstance/grey.css"));
    Style::repolish(this);

    connect(Core::getInstance(), &Core::fileTransferInfo, this, &FileTransferWidget::onFileTransferInfo);
    connect(Core::getInstance(), &Core::fileTransferAccepted, this, &FileTransferWidget::onFileTransferAccepted);
    connect(Core::getInstance(), &Core::fileTransferCancelled, this, &FileTransferWidget::onFileTransferCancelled);
    connect(Core::getInstance(), &Core::fileTransferPaused, this, &FileTransferWidget::onFileTransferPaused);
    connect(Core::getInstance(), &Core::fileTransferFinished, this, &FileTransferWidget::onFileTransferFinished);

    setupButtons();

    //preview
    if(fileInfo.direction == ToxFile::SENDING)
        showPreview(fileInfo.filePath);

    setFixedHeight(90);
}

FileTransferWidget::~FileTransferWidget()
{
    delete ui;
}

void FileTransferWidget::autoAcceptTransfer(const QString &path)
{
    QString filepath;
    int number = 0;

    QString suffix = QFileInfo(fileInfo.fileName).completeSuffix();
    QString base = QFileInfo(fileInfo.fileName).baseName();

    do
    {
        filepath = QString("%1/%2%3.%4").arg(path, base, number > 0 ? QString(" (%1)").arg(QString::number(number)) : QString(), suffix);
        number++;
    }
    while(QFileInfo(filepath).exists());

    //Do not automatically accept the file-transfer if the path is not writable.
    //The user can still accept it manually.
    if(isFilePathWritable(filepath))
        Core::getInstance()->acceptFileRecvRequest(fileInfo.friendId, fileInfo.fileNum, filepath);
    else
        qDebug() << "Warning: Cannot write to " << filepath;
}

void FileTransferWidget::acceptTransfer(const QString &filepath)
{
    //test if writable
    if(!isFilePathWritable(filepath))
    {
        QMessageBox::warning(0,
                             tr("Location not writable","Title of permissions popup"),
                             tr("You do not have permission to write that location. Choose another, or cancel the save dialog.", "text of permissions popup"));
        return;
    }

    //everything ok!
    Core::getInstance()->acceptFileRecvRequest(fileInfo.friendId, fileInfo.fileNum, filepath);
}

bool FileTransferWidget::isFilePathWritable(const QString &filepath)
{
    QFile tmp(filepath);
    bool writable = tmp.open(QIODevice::WriteOnly);
    tmp.remove();
    return writable;
}

void FileTransferWidget::onFileTransferInfo(ToxFile file)
{
    QTime now = QTime::currentTime();
    qint64 dt = lastTick.msecsTo(now); //ms

    if(fileInfo != file || dt < 1000)
        return;

    fileInfo = file;

    if(fileInfo.status == ToxFile::TRANSMITTING)
    {
        // update progress
        qreal progress = static_cast<qreal>(file.bytesSent) / static_cast<qreal>(file.filesize);
        ui->progressBar->setValue(static_cast<int>(progress * 100.0));

        // ETA, speed
        qreal deltaSecs = dt / 1000.0;

        qint64 deltaBytes = qMax(file.bytesSent - lastBytesSent, qint64(0));
        qreal bytesPerSec = static_cast<int>(static_cast<qreal>(deltaBytes) / deltaSecs);

        // calculate mean
        meanIndex = meanIndex % TRANSFER_ROLLING_AVG_COUNT;
        meanData[meanIndex++] = bytesPerSec;

        qreal meanBytesPerSec = 0.0;
        for(size_t i = 0; i < TRANSFER_ROLLING_AVG_COUNT; ++i)
            meanBytesPerSec += meanData[i];

        meanBytesPerSec /= static_cast<qreal>(TRANSFER_ROLLING_AVG_COUNT);

        // update UI
        if(meanBytesPerSec > 0)
        {
            // ETA
            QTime toGo = QTime(0,0).addSecs((file.filesize - file.bytesSent) / meanBytesPerSec);
            ui->etaLabel->setText(toGo.toString("hh:mm:ss"));
        }
        else
        {
            ui->etaLabel->setText("");
        }

        ui->progressLabel->setText(getHumanReadableSize(meanBytesPerSec) + "/s");

        lastBytesSent = file.bytesSent;
    }

    lastTick = now;

    // trigger repaint
    update();
}

void FileTransferWidget::onFileTransferAccepted(ToxFile file)
{
    if(fileInfo != file)
        return;

    fileInfo = file;

    setStyleSheet(Style::getStylesheet(":/ui/fileTransferInstance/yellow.css"));
    Style::repolish(this);

    setupButtons();
}

void FileTransferWidget::onFileTransferCancelled(ToxFile file)
{
    if(fileInfo != file)
        return;

    fileInfo = file;

    setStyleSheet(Style::getStylesheet(":/ui/fileTransferInstance/red.css"));
    Style::repolish(this);

    setupButtons();
    hideWidgets();

    disconnect(Core::getInstance(), 0, this, 0);
}

void FileTransferWidget::onFileTransferPaused(ToxFile file)
{
    if(fileInfo != file)
        return;

    fileInfo = file;

    ui->etaLabel->setText("");
    ui->progressLabel->setText(getHumanReadableSize(0) + "/s");

    // reset mean
    meanIndex = 0;
    for(size_t i=0; i<TRANSFER_ROLLING_AVG_COUNT; ++i)
        meanData[i] = 0.0;

    setStyleSheet(Style::getStylesheet(":/ui/fileTransferInstance/grey.css"));
    Style::repolish(this);

    setupButtons();
}

void FileTransferWidget::onFileTransferFinished(ToxFile file)
{
    if(fileInfo != file)
        return;

    fileInfo = file;

    setStyleSheet(Style::getStylesheet(":/ui/fileTransferInstance/green.css"));
    Style::repolish(this);

    setupButtons();
    hideWidgets();

    static const QStringList openExtensions = { "png", "jpeg", "jpg", "gif", "zip", "rar" };

    if(openExtensions.contains(QFileInfo(file.fileName).suffix()))
    {
        ui->bottomButton->setIcon(QIcon(":/ui/fileTransferInstance/browse_path.png"));
        ui->bottomButton->setObjectName("browse");
        ui->bottomButton->show();
    }

    // preview
    if(fileInfo.direction == ToxFile::RECEIVING)
        showPreview(fileInfo.filePath);

    disconnect(Core::getInstance(), 0, this, 0);
}

QString FileTransferWidget::getHumanReadableSize(qint64 size)
{
    static const char* suffix[] = {"B","kiB","MiB","GiB","TiB"};
    int exp = 0;

    if (size > 0)
        exp = std::min( (int) (log(size) / log(1024)), (int) (sizeof(suffix) / sizeof(suffix[0]) - 1));

    return QString().setNum(size / pow(1024, exp),'f', exp > 2 ? 2 : 0).append(suffix[exp]);
}

void FileTransferWidget::hideWidgets()
{
    ui->topButton->hide();
    ui->bottomButton->hide();
    ui->progressBar->hide();
    ui->progressLabel->hide();
    ui->etaLabel->hide();
}

void FileTransferWidget::setupButtons()
{
    switch(fileInfo.status)
    {
    case ToxFile::TRANSMITTING:
        ui->topButton->setIcon(QIcon(":/ui/fileTransferInstance/no_2x.png"));
        ui->topButton->setObjectName("cancel");

        ui->bottomButton->setIcon(QIcon(":/ui/fileTransferInstance/pause_2x.png"));
        ui->bottomButton->setObjectName("pause");
        break;
    case ToxFile::PAUSED:
        ui->topButton->setIcon(QIcon(":/ui/fileTransferInstance/no_2x.png"));
        ui->topButton->setObjectName("cancel");

        ui->bottomButton->setIcon(QIcon(":/ui/fileTransferInstance/arrow_white_2x.png"));
        ui->bottomButton->setObjectName("resume");
        break;
    case ToxFile::STOPPED:
    case ToxFile::BROKEN: //TODO: ?
        ui->topButton->setIcon(QIcon(":/ui/fileTransferInstance/no_2x.png"));
        ui->topButton->setObjectName("cancel");

        if(fileInfo.direction == ToxFile::SENDING)
        {
            ui->bottomButton->setIcon(QIcon(":/ui/fileTransferInstance/pause_2x.png"));
            ui->bottomButton->setObjectName("pause");
        }
        else
        {
            ui->bottomButton->setIcon(QIcon(":/ui/fileTransferInstance/yes_2x.png"));
            ui->bottomButton->setObjectName("accept");
        }
        break;
    }
}

void FileTransferWidget::handleButton(QPushButton *btn)
{
    if(fileInfo.direction == ToxFile::SENDING)
    {
        if(btn->objectName() == "cancel")
            Core::getInstance()->cancelFileSend(fileInfo.friendId, fileInfo.fileNum);
        else if(btn->objectName() == "pause")
            Core::getInstance()->pauseResumeFileSend(fileInfo.friendId, fileInfo.fileNum);
        else if(btn->objectName() == "resume")
            Core::getInstance()->pauseResumeFileSend(fileInfo.friendId, fileInfo.fileNum);
    }
    else
    {
        if(btn->objectName() == "cancel")
            Core::getInstance()->cancelFileRecv(fileInfo.friendId, fileInfo.fileNum);
        else if(btn->objectName() == "pause")
            Core::getInstance()->pauseResumeFileRecv(fileInfo.friendId, fileInfo.fileNum);
        else if(btn->objectName() == "resume")
            Core::getInstance()->pauseResumeFileRecv(fileInfo.friendId, fileInfo.fileNum);
        else if(btn->objectName() == "accept")
        {
            QString path = QFileDialog::getSaveFileName(0, tr("Save a file","Title of the file saving dialog"), QDir::home().filePath(fileInfo.fileName));
            acceptTransfer(path);
        }
    }

    if(btn->objectName() == "browse")
    {
        QDesktopServices::openUrl("file://" + fileInfo.filePath);
    }
}

void FileTransferWidget::showPreview(const QString &filename)
{
    static const QStringList previewExtensions = { "png", "jpeg", "jpg", "gif" };

    if(previewExtensions.contains(QFileInfo(filename).suffix()))
    {
        //QPixmap pmap = QPixmap(filename).scaled(QSize(ui->previewLabel->maximumWidth(), maximumHeight()), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPixmap pmap = QPixmap(filename).scaledToWidth(ui->previewLabel->maximumWidth(), Qt::SmoothTransformation);
        ui->previewLabel->setPixmap(pmap);
        ui->previewLabel->show();
    }
}

void FileTransferWidget::on_topButton_clicked()
{
    handleButton(ui->topButton);
}

void FileTransferWidget::on_bottomButton_clicked()
{
    handleButton(ui->bottomButton);
}
