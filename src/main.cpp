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
#include "src/profilelocker.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QMutexLocker>
#include <QProcess>
#include <opencv2/core/core_c.h>

#include <sodium.h>

#include "toxme.h"

#include <unistd.h>

#define EXIT_UPDATE_MACX 218 //We track our state using unique exit codes when debugging
#define EXIT_UPDATE_MACX_FAIL 216

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

int opencvErrorHandler(int status, const char* func_name, const char* err_msg,
                   const char* file_name, int line, void*)
{
    qWarning() << "OpenCV: ERROR ("<<status<<") in "
               <<file_name<<":"<<line<<":"<<func_name<<": "<<err_msg;
    return 0;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("qTox");
    a.setOrganizationName("Tox");
    a.setApplicationVersion("\nGit commit: " + QString(GIT_VERSION));
    
#ifdef HIGH_DPI
    a.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif

    qsrand(time(0));

    // Process arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("qTox, version: " + QString(GIT_VERSION) + "\nBuilt: " + __TIME__ + " " + __DATE__);
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("uri", QObject::tr("Tox URI to parse"));
    parser.addOption(QCommandLineOption("p", QObject::tr("Starts new instance and loads specified profile."), QObject::tr("profile")));
    parser.process(a);

#ifndef Q_OS_ANDROID
    IPC::getInstance();
#endif
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
        fprintf(stderr, "Couldn't open log file!\n");
        delete logFile;
        logFile = nullptr;
    }
#endif

    // Windows platform plugins DLL hell fix
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());
    a.addLibraryPath("platforms");

    qDebug() << "built on: " << __TIME__ << __DATE__ << "(" << TIMESTAMP << ")";
    qDebug() << "commit: " << GIT_VERSION << "\n";

    cvSetErrMode(CV_ErrModeParent);
    cvRedirectError(opencvErrorHandler);

#if defined(Q_OS_MACX) && defined(QT_RELEASE)
    if (qApp->applicationDirPath() != "/Applications/qtox.app/Contents/MacOS") {
        qDebug() << "OS X: Not in Applications folder";

        QMessageBox AskInstall;
        AskInstall.setIcon(QMessageBox::Question);
        AskInstall.setWindowModality(Qt::ApplicationModal);
        AskInstall.setText("Move to Applications folder?");
        AskInstall.setInformativeText("I can move myself to the Applications folder, keeping your downloads folder less cluttered.\r\n");
        AskInstall.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        AskInstall.setDefaultButton(QMessageBox::Yes);

        int AskInstallAttempt = AskInstall.exec(); //Actually ask the user

        if (AskInstallAttempt == QMessageBox::Yes) {
            QProcess *sudoprocess = new QProcess;
            QProcess *qtoxprocess = new QProcess;

            QString bindir = qApp->applicationDirPath();
            QString appdir = bindir;
            appdir.chop(15);
            QString sudo = bindir + "/qtox_sudo rsync -avzhpltK " + appdir + " /Applications";
            QString qtox = "open /Applications/qtox.app";

            QString appdir_noqtox = appdir;
            appdir_noqtox.chop(8);

            if ((appdir_noqtox + "qtox.app") != appdir) //quick safety check
            {
                qDebug() << "OS X: Attmepted to delete non qTox directory!";
                return EXIT_UPDATE_MACX_FAIL;
            }

            QDir old_app(appdir);

            sudoprocess->start(sudo); //Where the magic actually happens, safety checks ^
            sudoprocess->waitForFinished();

            if (old_app.removeRecursively()) //We've just deleted the running program
            {
                qDebug() << "OS X: Cleaned up old directory";
            }
            else
            {
                qDebug() << "OS X: This should never happen, the directory failed to delete";
            }

            if (fork() != 0) //Forking is required otherwise it won't actually cleanly launch
                return EXIT_UPDATE_MACX;

            qtoxprocess->start(qtox);

            return 0; //Actually kills it
        }
    }
#endif

    // Install Unicode 6.1 supporting font
    QFontDatabase::addApplicationFont("://DejaVuSans.ttf");

    // Check whether we have an update waiting to be installed
#if AUTOUPDATE_ENABLED
    if (AutoUpdater::isLocalUpdateReady())
        AutoUpdater::installLocalUpdate(); ///< NORETURN
#endif


#ifndef Q_OS_ANDROID
    // Inter-process communication
    IPC& ipc = IPC::getInstance();
    ipc.registerEventHandler("uri", &toxURIEventHandler);
    ipc.registerEventHandler("save", &toxSaveEventHandler);
    ipc.registerEventHandler("activate", &toxActivateEventHandler);

    // If we're the IPC owner and we just started, then
    // either we're the only running instance or any other instance
    // is already so frozen it lost ownership.
    // It's safe to remove any potential stale locks in this situation.
    if (ipc.isCurrentOwner())
        ProfileLocker::clearAllLocks();

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
                time_t event = ipc.postEvent("uri", firstParam.toUtf8());
                ipc.waitUntilAccepted(event);
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
                time_t event = ipc.postEvent("save", firstParam.toUtf8());
                ipc.waitUntilAccepted(event);
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
        uint32_t dest = 0;
        if (parser.isSet("p"))
            dest = Settings::getInstance().getCurrentProfileId();

        time_t event = ipc.postEvent("activate", QByteArray(), dest);
        if (ipc.waitUntilAccepted(event, 2))
        {
            if (!ipc.isCurrentOwner())
                return EXIT_SUCCESS;
        }
    }
#endif

    Nexus::getInstance().start();

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
