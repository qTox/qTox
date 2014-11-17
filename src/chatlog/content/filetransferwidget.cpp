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
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
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
    ui->progressLabel->setText("0%");
    ui->etaLabel->setText("--:--");

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

    setFixedHeight(85);
}

FileTransferWidget::~FileTransferWidget()
{
    delete ui;
}

void FileTransferWidget::onFileTransferInfo(ToxFile file)
{
    if(fileInfo != file)
        return;

    fileInfo = file;

    // update progress
    qreal progress = static_cast<qreal>(file.bytesSent) / static_cast<qreal>(file.filesize);
    ui->progressBar->setValue(static_cast<int>(progress * 100.0));

    // eta, speed
    QTime now = QTime::currentTime();
    qreal deltaSecs = lastTick.msecsTo(now) / 1000.0;

    if(deltaSecs >= 1.0)
    {
        qint64 deltaBytes = file.bytesSent - lastBytesSent;
        qint64 bytesPerSec = static_cast<int>(static_cast<qreal>(deltaBytes) / deltaSecs);

        if(bytesPerSec > 0)
        {
            QTime toGo(0,0,file.filesize / bytesPerSec);
            ui->etaLabel->setText(toGo.toString("mm:ss"));
        }
        else
        {
            ui->etaLabel->setText("--:--");
        }

        ui->progressLabel->setText(getHumanReadableSize(bytesPerSec) + "/s");

        lastTick = now;
        lastBytesSent = file.bytesSent;
    }

    setupButtons();

    repaint();
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

    return QString().setNum(size / pow(1024, exp),'f',2).append(suffix[exp]);
}

void FileTransferWidget::hideWidgets()
{
    ui->buttonWidget->hide();
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
        if(btn->objectName() == "pause")
            Core::getInstance()->pauseResumeFileSend(fileInfo.friendId, fileInfo.fileNum);
        if(btn->objectName() == "resume")
            Core::getInstance()->pauseResumeFileSend(fileInfo.friendId, fileInfo.fileNum);
    }
    else
    {
        if(btn->objectName() == "cancel")
            Core::getInstance()->cancelFileRecv(fileInfo.friendId, fileInfo.fileNum);
        if(btn->objectName() == "pause")
            Core::getInstance()->pauseResumeFileRecv(fileInfo.friendId, fileInfo.fileNum);
        if(btn->objectName() == "resume")
            Core::getInstance()->pauseResumeFileRecv(fileInfo.friendId, fileInfo.fileNum);
        if(btn->objectName() == "accept")
        {
            QString path = QFileDialog::getSaveFileName(0, tr("Save a file","Title of the file saving dialog"), QDir::home().filePath(fileInfo.fileName));

            if(!QFileInfo(QDir(path).path()).isWritable())
            {
                QMessageBox::warning(0,
                                     tr("Location not writable","Title of permissions popup"),
                                     tr("You do not have permission to write that location. Choose another, or cancel the save dialog.", "text of permissions popup"));

                return;
            }

            if(!path.isEmpty())
                Core::getInstance()->acceptFileRecvRequest(fileInfo.friendId, fileInfo.fileNum, path);
        }
    }
}

void FileTransferWidget::showPreview(const QString &filename)
{
    //QPixmap pmap = QPixmap(filename).scaled(QSize(ui->previewLabel->maximumWidth(), maximumHeight()), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPixmap pmap = QPixmap(filename).scaledToWidth(ui->previewLabel->maximumWidth(), Qt::SmoothTransformation);
    ui->previewLabel->setPixmap(pmap);
    ui->previewLabel->show();
}

void FileTransferWidget::on_topButton_clicked()
{
    handleButton(ui->topButton);
}

void FileTransferWidget::on_bottomButton_clicked()
{
    handleButton(ui->bottomButton);
}
