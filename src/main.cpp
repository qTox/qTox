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

#include "widget/widget.h"
#include "misc/settings.h"
#include "src/nexus.h"
#include "src/ipc.h"
#include "src/widget/toxuri.h"
#include "src/widget/toxsave.h"
#include "src/autoupdate.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QMutexLocker>

#include <sodium.h>

#ifdef LOG_TO_FILE
static QtMessageHandler dflt;
static QTextStream* logFile {nullptr};
static QMutex mutex;

void myMessageHandler(QtMsgType type, const QMessageLogContext& ctxt, const QString& msg)
{
    if (!logFile)
        return;

    // Silence qWarning spam due to bug in QTextBrowser (trying to open a file for base64 images)
    if (ctxt.function == QString("virtual bool QFSFileEngine::open(QIODevice::OpenMode)")
            && msg == QString("QFSFileEngine::open: No file name specified"))
        return;

    QMutexLocker locker(&mutex);
    dflt(type, ctxt, msg);
    *logFile << QTime::currentTime().toString("HH:mm:ss' '") << msg << '\n';
    logFile->flush();
}
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("qTox");
    a.setOrganizationName("Tox");
    a.setApplicationVersion("\nGit commit: " + QString(GIT_VERSION));

    // Process arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("qTox, version: " + QString(GIT_VERSION) + "\nBuilt: " + __TIME__ + " " + __DATE__);
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("uri", QObject::tr("Tox URI to parse"));
    parser.addOption(QCommandLineOption("p", QObject::tr("Starts new instance and loads specified profile."), QObject::tr("profile")));
    parser.process(a);

    Settings::getInstance(); // Build our Settings singleton as soon as QApplication is ready, not before

    if (parser.isSet("p"))
    {
        QString profile = parser.value("p");
        if (QDir(Settings::getSettingsDirPath()).exists(profile + ".tox"))
        {
            qDebug() << "Setting profile to" << profile;
            Settings::getInstance().switchProfile(profile);
        }
        else
        {
            qWarning() << "Error: -p profile" << profile + ".tox" << "doesn't exist";
            return EXIT_FAILURE;
        }
    }

    sodium_init(); // For the auto-updater

#ifdef LOG_TO_FILE
    logFile = new QTextStream;
    dflt = qInstallMessageHandler(nullptr);
    QFile logfile(QDir(Settings::getSettingsDirPath()).filePath("qtox.log"));
    if (logfile.open(QIODevice::Append))
    {
        logFile->setDevice(&logfile);
        *logFile << QDateTime::currentDateTime().toString("\nyyyy-MM-dd HH:mm:ss' file logger starting\n'");
        qInstallMessageHandler(myMessageHandler);
    }
    else
    {
        fprintf(stderr, "Couldn't open log file!!!\n");
        delete logFile;
        logFile = nullptr;
    }
#endif

    // Windows platform plugins DLL hell fix
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());
    a.addLibraryPath("platforms");

    qDebug() << "built on: " << __TIME__ << __DATE__ << "(" << TIMESTAMP << ")";
    qDebug() << "commit: " << GIT_VERSION << "\n";

    // Install Unicode 6.1 supporting font
    QFontDatabase::addApplicationFont("://DejaVuSans.ttf");

    // Check whether we have an update waiting to be installed
#if AUTOUPDATE_ENABLED
    if (AutoUpdater::isLocalUpdateReady())
        AutoUpdater::installLocalUpdate(); ///< NORETURN
#endif

    Nexus::getInstance().start();

#ifndef Q_OS_ANDROID
    // Inter-process communication
    IPC ipc;
    ipc.registerEventHandler(&toxURIEventHandler);
    ipc.registerEventHandler(&toxSaveEventHandler);
    ipc.registerEventHandler(&toxActivateEventHandler);

    if (parser.positionalArguments().size() > 0)
    {
        QString firstParam(parser.positionalArguments()[0]);
        // Tox URIs. If there's already another qTox instance running, we ask it to handle the URI and we exit
        // Otherwise we start a new qTox instance and process it ourselves
        if (firstParam.startsWith("tox:"))
        {
            if (ipc.isCurrentOwner()) // Don't bother sending an event if we're going to process it ourselves
            {
                handleToxURI(firstParam.toUtf8());
            }
            else
            {
                time_t event = ipc.postEvent(firstParam.toUtf8());
                ipc.waitUntilProcessed(event);
                // If someone else processed it, we're done here, no need to actually start qTox
                if (!ipc.isCurrentOwner())
                    return EXIT_SUCCESS;
            }
        }
        else if (firstParam.endsWith(".tox"))
        {
            if (ipc.isCurrentOwner()) // Don't bother sending an event if we're going to process it ourselves
            {
                handleToxSave(firstParam.toUtf8());
            }
            else
            {
                time_t event = ipc.postEvent(firstParam.toUtf8());
                ipc.waitUntilProcessed(event);
                // If someone else processed it, we're done here, no need to actually start qTox
                if (!ipc.isCurrentOwner())
                    return EXIT_SUCCESS;
            }
        }
        else
        {
            fprintf(stderr, "Invalid argument\n");
            return EXIT_FAILURE;
        }
    }
    else if (!ipc.isCurrentOwner() && !parser.isSet("p"))
    {
        time_t event = ipc.postEvent("$activate");
        ipc.waitUntilProcessed(event);
        if (!ipc.isCurrentOwner())
            return EXIT_SUCCESS;
    }
#endif

    // Run
    a.setQuitOnLastWindowClosed(false);
    int errorcode = a.exec();

#ifdef LOG_TO_FILE
    delete logFile;
    logFile = nullptr;
#endif

    Nexus::destroyInstance();

    return errorcode;
}
