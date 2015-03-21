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


#include "widget.h"
#include "ui_widget.h"

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QMessageBox>
#include <QMetaObject>

#include "settingsDir.h"
#include "update.h"

#ifdef Q_OS_WIN
const bool supported = true;
const QString QTOX_PATH = "qtox.exe";
#else
const bool supported = false;
const QString QTOX_PATH;
#endif

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    // Updates only for supported platforms
    if (!supported)
        fatalError(tr("The qTox updater is not supported on this platform."));

    QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::setProgress(int value)
{
    ui->progress->setValue(value);
    ui->progress->repaint();
    qApp->processEvents();
}

void Widget::fatalError(QString message)
{
    QMessageBox::critical(this,tr("Error"), message+'\n'+tr("qTox will restart now."));
    deleteUpdate();
    restoreBackups();
    startQToxAndExit();
}

void Widget::deleteUpdate()
{
    QDir updateDir(getSettingsDirPath()+"/update/");
    updateDir.removeRecursively();
}

void Widget::startQToxAndExit()
{
    QProcess::startDetached(QTOX_PATH);
    exit(0);
}

void Widget::deleteBackups()
{
    for (QString file : backups)
        QFile(file+".bak").remove();
}

void Widget::restoreBackups()
{
    for (QString file : backups)
        QFile(file+".bak").rename(file);
}

void Widget::update()
{
    /// 1. Find and parse the update (0-5%)
    // Check that the dir exists
    QString updateDirStr = getSettingsDirPath()+"/update/";
    QDir updateDir(updateDirStr);
    if (!updateDir.exists())
        fatalError(tr("No update found."));

    // Check that we have a flist and that every file on the diff exists
    QFile updateFlistFile(updateDirStr+"flist");
    if (!updateFlistFile.open(QIODevice::ReadOnly))
        fatalError(tr("The update is incomplete."));

    QByteArray updateFlistData = updateFlistFile.readAll();
    updateFlistFile.close();

    setProgress(1);

    QList<UpdateFileMeta> updateFlist = parseFlist(updateFlistData);
    setProgress(2);
    QList<UpdateFileMeta> diff = genUpdateDiff(updateFlist);
    setProgress(4);
    for (UpdateFileMeta fileMeta : diff)
        if (!QFile::exists(updateDirStr+fileMeta.installpath))
            fatalError(tr("The update is incomplete."));

    if (diff.size() == 0)
       fatalError(tr("The diff list is empty."));

    setProgress(5);

    /// 2. Check the update (5-50%)
    float checkProgressStep = 45.0/(float)diff.size();
    float checkProgress = 5;
    for (UpdateFileMeta fileMeta : diff)
    {
        UpdateFile file;
        file.metadata = fileMeta;

        QFile fileFile(updateDirStr+fileMeta.installpath);
        if (!fileFile.open(QIODevice::ReadOnly))
            fatalError(tr("Update files are unreadable."));

        file.data = fileFile.readAll();
        fileFile.close();

        if (file.data.size() != (int)fileMeta.size)
            fatalError(tr("Update files are corrupted."));

        if (crypto_sign_verify_detached(file.metadata.sig, (unsigned char*)file.data.data(),
                                        file.data.size(), key) != 0)
            fatalError(tr("Update files are corrupted."));

        checkProgress += checkProgressStep;
        setProgress(checkProgress);
    }
    setProgress(50);

    /// 3. Install the update (50-95%)
    float installProgressStep = 45.0/(float)diff.size();
    float installProgress = 50;
    for (UpdateFileMeta fileMeta : diff)
    {
        // Backup old files
        if (QFile(fileMeta.installpath).exists())
        {
            QFile(fileMeta.installpath+".bak").remove();
            QFile(fileMeta.installpath).rename(fileMeta.installpath+".bak");
            backups.append(fileMeta.installpath);
        }

        // Install new ones
        QDir().mkpath(QFileInfo(fileMeta.installpath).absolutePath());
        QFile fileFile(updateDirStr+fileMeta.installpath);
        if (!fileFile.copy(fileMeta.installpath))
            fatalError(tr("Unable to copy the update's files from ")+(updateDirStr+fileMeta.installpath)+" to "+fileMeta.installpath);
        installProgress += installProgressStep;
        setProgress(installProgress);
    }
    setProgress(95);

    /// 4. Delete the update and backups (95-100%)
    deleteUpdate();
    setProgress(97);
    deleteBackups();
    setProgress(100);

    /// 5. Start qTox and exit
    startQToxAndExit();
}
