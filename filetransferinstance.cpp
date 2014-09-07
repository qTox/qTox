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

#include "filetransferinstance.h"
#include "widget/widget.h"
#include "core.h"
#include "math.h"
#include "style.h"
#include <QFileDialog>
#include <QPixmap>
#include <QPainter>
#include <QMessageBox>
#include <QBuffer>

uint FileTransferInstance::Idconter = 0;

FileTransferInstance::FileTransferInstance(ToxFile File)
    : lastUpdate{QDateTime::currentDateTime()}, lastBytesSent{0},
      fileNum{File.fileNum}, friendId{File.friendId}, direction{File.direction}
{
    id = Idconter++;
    state = tsPending;

    filename = File.fileName;
    size = getHumanReadableSize(File.filesize);
    speed = "0B/s";
    eta = "00:00";
    if (File.direction == ToxFile::SENDING)
    {
        QImage preview;
        File.file->seek(0);
        if (preview.loadFromData(File.file->readAll()))
        {
            pic = preview.scaledToHeight(40);
        }
        File.file->seek(0);
    }
}

QString FileTransferInstance::getHumanReadableSize(unsigned long long size)
{
    static const char* suffix[] = {"B","kiB","MiB","GiB","TiB"};
    int exp = 0;
    if (size)
        exp = std::min( (int) (log(size) / log(1024)), (int) (sizeof(suffix) / sizeof(suffix[0]) - 1));
    return QString().setNum(size / pow(1024, exp),'f',2).append(suffix[exp]);
}

void FileTransferInstance::onFileTransferInfo(int FriendId, int FileNum, int64_t Filesize, int64_t BytesSent, ToxFile::FileDirection Direction)
{
    if (FileNum != fileNum || FriendId != friendId || Direction != direction)
            return;

//    state = tsProcessing;
    QDateTime newtime = QDateTime::currentDateTime();
    int timediff = lastUpdate.secsTo(newtime);
    if (timediff <= 0)
        return;
    qint64 diff = BytesSent - lastBytesSent;
    if (diff < 0)
    {
        qWarning() << "FileTransferInstance::onFileTransferInfo: Negative transfer speed !";
        diff = 0;
    }
    long rawspeed = diff / timediff;
    speed = getHumanReadableSize(rawspeed)+"/s";
    size = getHumanReadableSize(Filesize);
    if (!rawspeed)
        return;
    int etaSecs = (Filesize - BytesSent) / rawspeed;
    QTime etaTime(0,0);
    etaTime = etaTime.addSecs(etaSecs);
    eta = etaTime.toString("mm:ss");
    lastUpdate = newtime;
    lastBytesSent = BytesSent;
    emit stateUpdated();
}

void FileTransferInstance::onFileTransferCancelled(int FriendId, int FileNum, ToxFile::FileDirection Direction)
{
    if (FileNum != fileNum || FriendId != friendId || Direction != direction)
            return;
    disconnect(Widget::getInstance()->getCore(),0,this,0);
    state = tsCanceled;

    emit stateUpdated();
}

void FileTransferInstance::onFileTransferFinished(ToxFile File)
{
    if (File.fileNum != fileNum || File.friendId != friendId || File.direction != direction)
            return;
    disconnect(Widget::getInstance()->getCore(),0,this,0);

    if (File.direction == ToxFile::RECEIVING)
    {
        QImage preview;
        QFile previewFile(File.filePath);
        if (previewFile.open(QIODevice::ReadOnly) && previewFile.size() <= 1024*1024*25) // Don't preview big (>25MiB) images
        {
            if (preview.loadFromData(previewFile.readAll()))
            {
                pic = preview.scaledToHeight(40);
            }
            previewFile.close();
        }
    }

    state = tsFinished;

    emit stateUpdated();
}

void FileTransferInstance::cancelTransfer()
{
    Widget::getInstance()->getCore()->cancelFileSend(friendId, fileNum);
    state = tsCanceled;
    emit stateUpdated();
}

void FileTransferInstance::rejectRecvRequest()
{
    Widget::getInstance()->getCore()->rejectFileRecvRequest(friendId, fileNum);
    onFileTransferCancelled(friendId, fileNum, direction);
    state = tsCanceled;
    emit stateUpdated();
}

// for whatever the fuck reason, QFileInfo::isWritable() always fails for files that don't exist
// which makes it useless for our case
// since QDir doesn't have an isWritable(), the only option I can think of is to make/delete the file
// surely this is a common problem that has a qt-implemented solution?
bool isFileWritable(QString& path)
{
    QFile file(path);
    bool exists = file.exists();
    bool out = file.open(QIODevice::WriteOnly);
    file.close();
    if (!exists)
        file.remove();
    return out;
}

void FileTransferInstance::acceptRecvRequest()
{
    QString path;
    while (true)
    {
        path = QFileDialog::getSaveFileName(Widget::getInstance(), tr("Save a file","Title of the file saving dialog"), QDir::current().filePath(filename));
        if (path.isEmpty())
            return;
        else
        {
            //bool savable = QFileInfo(path).isWritable();
            //qDebug() << path << " is writable: " << savable;
            //qDebug() << "/home/bill/bliss.pdf writable: " << QFileInfo("/home/bill/bliss.pdf").isWritable();
            if (isFileWritable(path))
                break;
            else
                QMessageBox::warning(Widget::getInstance(), tr("Location not writable","Title of permissions popup"), tr("You do not have permission to write that location. Choose another, or cancel the save dialog.", "text of permissions popup"));
        }
    }

    savePath = path;

    Widget::getInstance()->getCore()->acceptFileRecvRequest(friendId, fileNum, path);
    state = tsProcessing;

    emit stateUpdated();
}

void FileTransferInstance::pauseResumeRecv()
{
    Widget::getInstance()->getCore()->pauseResumeFileRecv(friendId, fileNum);
    if (state == tsProcessing)
        state = tsPaused;
    else state = tsProcessing;
    emit stateUpdated();
}

void FileTransferInstance::pauseResumeSend()
{
    Widget::getInstance()->getCore()->pauseResumeFileSend(friendId, fileNum);
    if (state == tsProcessing)
        state = tsPaused;
    else state = tsProcessing;
    emit stateUpdated();
}

QString FileTransferInstance::QImage2base64(const QImage &img)
{
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "PNG"); // writes image into ba in PNG format
    return ba.toBase64();
}

QString FileTransferInstance::getHtmlImage()
{
    qDebug() << "QString FileTransferInstance::getHtmlImage()";

    QString res;
    if (state == tsPending || state == tsProcessing || state == tsPaused)
    {
        QImage leftBtnImg(":/ui/fileTransferInstance/stopFileButton.png");
        QImage rightBtnImg;
        if (state == tsProcessing)
            rightBtnImg = QImage(":/ui/fileTransferInstance/pauseFileButton.png");
        else if (state == tsPaused)
            rightBtnImg = QImage(":/ui/fileTransferInstance/resumeFileButton.png");
        else
            rightBtnImg = QImage(":/ui/fileTransferInstance/acceptFileButton.png");

        res = draw2ButtonsForm("green", leftBtnImg, rightBtnImg);
    } else if (state == tsCanceled)
    {
        res = drawButtonlessForm("red");
    } else if (state == tsFinished)
    {
        res = drawButtonlessForm("green");
    }

    return res;
}

void FileTransferInstance::pressFromHtml(QString code)
{
    if (state == tsFinished || state == tsCanceled)
        return;

    if (direction == ToxFile::SENDING)
    {
        if (code == "btnA")
            cancelTransfer();
        else if (code == "btnB")
            pauseResumeSend();
    } else {
        if (code == "btnA")
            rejectRecvRequest();
        else if (code == "btnB")
        {
            if (state == tsPending)
                acceptRecvRequest();
            else
                pauseResumeRecv();
        }
    }
}

QString FileTransferInstance::drawButtonlessForm(const QString &type)
{
    QString imgAStr;
    QString imgBStr;

    if (type == "red")
    {
        imgAStr = "<img src=\"data:placeholder/png;base64," + QImage2base64(QImage(":/ui/fileTransferInstance/emptyLRedFileButton.png")) + "\">";
        imgBStr = "<img src=\"data:placeholder/png;base64," + QImage2base64(QImage(":/ui/fileTransferInstance/emptyRRedFileButton.png")) + "\">";
    } else {
        imgAStr = "<img src=\"data:placeholder/png;base64," + QImage2base64(QImage(":/ui/fileTransferInstance/emptyLGreenFileButton.png")) + "\">";
        imgBStr = "<img src=\"data:placeholder/png;base64," + QImage2base64(QImage(":/ui/fileTransferInstance/emptyRGreenFileButton.png")) + "\">";
    }

    QString content = "<p>" + filename + "</p><p>" + size + "</p>";

    return wrapIntoForm(content, type, imgAStr, imgBStr);
}

QString FileTransferInstance::insertMiniature()
{
    if (pic == QImage())
        return QString();

    QString widgetId = QString::number(getId());

    QString res;
    res  = "<td>\n";
    res += "<img src=\"data:mini." + widgetId + "/png;base64," + QImage2base64(pic) + "\">";
    res += "</td>\n";
    return res;
}

QString FileTransferInstance::draw2ButtonsForm(const QString &type, const QImage &imgA, const QImage &imgB)
{
    QString widgetId = QString::number(getId());
    QString imgAstr = "<img src=\"data:ftrans." + widgetId + ".btnA/png;base64," + QImage2base64(imgA) + "\">";
    QString imgBstr = "<img src=\"data:ftrans." + widgetId + ".btnB/png;base64," + QImage2base64(imgB) + "\">";

    QString content;
    content += "<p>" + filename + "</p>";
    content += "<p>" + getHumanReadableSize(lastBytesSent) + " / " + size + "&nbsp;(" + speed + " ETA: " + eta + ")</p>\n";

    return wrapIntoForm(content, type, imgAstr, imgBstr);
}

QString FileTransferInstance::wrapIntoForm(const QString& content, const QString &type, const QString &imgAstr, const QString &imgBstr)
{
    QString res;

    res =  "<table widht=100% cellspacing=\"1\">\n";
    res += "<tr valign=middle>\n";
    res += "<td>\n";
    res += "<div class=button>" + imgAstr + "</div>\n";
    res += "</td>\n";
    res += insertMiniature();
    res += "<td width=100%>\n";
    res += "<div class=" + type + ">";
    res += content;
    res += "</div>\n";
    res += "</td>\n";
    res += "<td>\n";
    res += "<div class=button>" + imgBstr + "</div>\n";
    res += "</td>\n";
    res += "</tr>\n";
    res += "</table>\n";

    return res;
}
