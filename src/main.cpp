/*
    Copyright © 2014-2015 by The qTox Project

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "widget/widget.h"
#include "persistence/settings.h"
#include "src/nexus.h"
#include "src/ipc.h"
#include "src/net/toxuri.h"
#include "src/net/autoupdate.h"
#include "src/persistence/toxsave.h"
#include "src/persistence/profile.h"
#include "src/widget/loginscreen.h"
#include "src/widget/translator.h"
#include "src/video/camerasource.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QMutexLocker>

#include <sodium.h>
#include <fcntl.h>

#if defined(Q_OS_OSX)
#include "platform/install_osx.h"
#endif

#ifdef LOG_TO_FILE
static int logFileFileNO = -1;
#endif

void logMessageHandler(QtMsgType type, const QMessageLogContext& ctxt, const QString& msg)
{
    // Silence qWarning spam due to bug in QTextBrowser (trying to open a file for base64 images)
    if (ctxt.function == QString("virtual bool QFSFileEngine::open(QIODevice::OpenMode)")
            && msg == QString("QFSFileEngine::open: No file name specified"))
        return;

    QByteArray LogMsg = (qFormatLogMessage(type, ctxt, msg) + "\n").toLocal8Bit();
    write(STDERR_FILENO, LogMsg.constData(), LogMsg.size());

#ifdef LOG_TO_FILE
    if (logFileFileNO < 0)
        return;

    write(logFileFileNO, LogMsg.constData(), LogMsg.size());
#endif
}

int main(int argc, char *argv[])
{
    qSetMessagePattern("%{time [HH:mm:ss.zzz]} %{file}:%{line} %{type} %{message}");
    qInstallMessageHandler(logMessageHandler);

    QApplication a(argc, argv);
    a.setApplicationName("qTox");
    a.setOrganizationName("Tox");
    a.setApplicationVersion("\nGit commit: " + QString(GIT_VERSION));

#if defined(Q_OS_OSX)
    //osx::moveToAppFolder(); TODO: Add setting to enable this feature.
    osx::migrateProfiles();
#endif

#ifdef HIGH_DPI
    a.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif

    qsrand(time(0));
    Settings::getInstance();
    Translator::translate();

    // Process arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("qTox, version: " + QString(GIT_VERSION) + "\nBuilt: " + __TIME__ + " " + __DATE__);
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("uri", QObject::tr("Tox URI to parse"));
    parser.addOption(QCommandLineOption("p", QObject::tr("Starts new instance and loads specified profile."), QObject::tr("profile")));
    parser.process(a);

#ifndef Q_OS_ANDROID
    IPC& ipc = IPC::getInstance();
#endif

    sodium_init(); // For the auto-updater

#ifdef LOG_TO_FILE
    QString logFileDir = Settings::getInstance().getSettingsDirPath();
    QString logfile = logFileDir + "qtox.log";
    logFileFileNO = open(logfile.toLocal8Bit().constData(), O_WRONLY | O_APPEND | O_CREAT);

    if (QFileInfo(logfile).size() > 1000000) {
        qDebug() << "Log file over 1MB, rotating...";
		
        QDir dir (logFileDir);
        // Check if log.1 already exists, and if so, delete it
        if (dir.remove(logFileDir + "qtox.log.1")) {
            qDebug() << "Removed log successfully";
        } else {
            qDebug() << "Unable to remove old log file";
        }

        dir.rename(logFileDir + "qtox.log", logFileDir + "qtox.log.1");

        int oldLogFileFileNO = logFileFileNO;
        logFileFileNO = open(logfile.toLocal8Bit().constData(), O_WRONLY | O_APPEND | O_CREAT);
        close(oldLogFileFileNO);
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


    QString profileName;
    bool autoLogin = Settings::getInstance().getAutoLogin();
#ifndef Q_OS_ANDROID
    // Inter-process communication
    ipc.registerEventHandler("uri", &toxURIEventHandler);
    ipc.registerEventHandler("save", &toxSaveEventHandler);
    ipc.registerEventHandler("activate", &toxActivateEventHandler);

    uint32_t ipcDest = 0;
    QString eventType, firstParam;
    if (parser.isSet("p"))
    {
        profileName = parser.value("p");
        if (!Profile::exists(profileName))
        {
            qCritical() << "-p profile" << profileName + ".tox" << "doesn't exist";
            return EXIT_FAILURE;
        }
        ipcDest = Settings::makeProfileId(profileName);
        autoLogin = true;
    }
    else
        profileName = Settings::getInstance().getCurrentProfile();

    if (parser.positionalArguments().size() == 0)
        eventType = "activate";
    else
    {
        firstParam = parser.positionalArguments()[0];
        // Tox URIs. If there's already another qTox instance running, we ask it to handle the URI and we exit
        // Otherwise we start a new qTox instance and process it ourselves
        if (firstParam.startsWith("tox:"))
            eventType = "uri";
        else if (firstParam.endsWith(".tox"))
            eventType = "save";
        else
        {
            qCritical() << "Invalid argument";
            return EXIT_FAILURE;
        }
    }

    if (!ipc.isCurrentOwner())
    {
        time_t event = ipc.postEvent(eventType, firstParam.toUtf8(), ipcDest);
        // If someone else processed it, we're done here, no need to actually start qTox
        if (ipc.waitUntilAccepted(event, 2))
        {
            qDebug() << "Event" << eventType << "was handled by other client.";
            return EXIT_SUCCESS;
        }
    }
#endif

    // Autologin
    if (autoLogin)
    {
        if (Profile::exists(profileName))
        {
            if (!Profile::isEncrypted(profileName))
            {
                Profile* profile = Profile::loadProfile(profileName);
                if (profile)
                    Nexus::getInstance().setProfile(profile);
            }
            Settings::getInstance().setCurrentProfile(profileName);
        }
    }

    Nexus::getInstance().start();

#ifndef Q_OS_ANDROID
    // Event was not handled by already running instance therefore we handle it ourselves
    if (eventType == "uri")
        handleToxURI(firstParam.toUtf8());
    else if (eventType == "save")
        handleToxSave(firstParam.toUtf8());
#endif

    // Run
    int errorcode = a.exec();

#ifdef LOG_TO_FILE
    close(logFileFileNO);
#endif

    Nexus::destroyInstance();
    CameraSource::destroyInstance();
    Settings::destroyInstance();
    qDebug() << "Clean exit with status" << errorcode;
    return errorcode;
}
