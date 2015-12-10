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
#include <QFile>
#include <QFontDatabase>
#include <QMutexLocker>

#include <sodium.h>

#if defined(Q_OS_MACX) && defined(QT_RELEASE)
#include "platform/install_osx.h"
#endif

#ifdef LOG_TO_FILE
static std::unique_ptr<QTextStream> logFileStream {nullptr};
static std::unique_ptr<QFile> logFileFile {nullptr};
static QMutex mutex;
#endif

void logMessageHandler(QtMsgType type, const QMessageLogContext& ctxt, const QString& msg)
{
    // Silence qWarning spam due to bug in QTextBrowser (trying to open a file for base64 images)
    if (ctxt.function == QString("virtual bool QFSFileEngine::open(QIODevice::OpenMode)")
            && msg == QString("QFSFileEngine::open: No file name specified"))
        return;

    QString LogMsg = QString("[%1] %2:%3 : ")
            .arg(QTime::currentTime().toString("HH:mm:ss.zzz")).arg(ctxt.file).arg(ctxt.line);
    switch (type)
    {
        case QtDebugMsg:
            LogMsg += "Debug";
            break;
        case QtWarningMsg:
            LogMsg += "Warning";
            break;
        case QtCriticalMsg:
            LogMsg += "Critical";
            break;
        case QtFatalMsg:
            LogMsg += "Fatal";
            break;
        default:
            break;
    }

    LogMsg += ": " + msg + "\n";

    QTextStream out(stderr, QIODevice::WriteOnly);
    out << LogMsg;

#ifdef LOG_TO_FILE
    if (!logFileStream)
        return;

    QMutexLocker locker(&mutex);
    *logFileStream << LogMsg;
    logFileStream->flush();
#endif
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(logMessageHandler); // Enable log as early as possible
    QApplication a(argc, argv);
    a.setApplicationName("qTox");
    a.setOrganizationName("Tox");
    a.setApplicationVersion("\nGit commit: " + QString(GIT_VERSION));

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
    logFileStream.reset(new QTextStream);
    logFileFile.reset(new QFile(Settings::getInstance().getSettingsDirPath()+"qtox.log"));
    if (logFileFile->open(QIODevice::Append))
    {
        logFileStream->setDevice(logFileFile.get());
        *logFileStream << QDateTime::currentDateTime().toString("\nyyyy-MM-dd HH:mm:ss' qTox file logger starting\n'");
    }
    else
    {
        qWarning() << "Couldn't open log file!\n";
        logFileStream.release();
    }
#endif

    // Windows platform plugins DLL hell fix
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());
    a.addLibraryPath("platforms");

    qDebug() << "built on: " << __TIME__ << __DATE__ << "(" << TIMESTAMP << ")";
    qDebug() << "commit: " << GIT_VERSION << "\n";

#if defined(Q_OS_MACX) && defined(QT_RELEASE)
    osx::moveToAppFolder();
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
    ipc.registerEventHandler("uri", &toxURIEventHandler);
    ipc.registerEventHandler("save", &toxSaveEventHandler);
    ipc.registerEventHandler("activate", &toxActivateEventHandler);

    if (parser.isSet("p"))
    {
        QString profileName = parser.value("p");
        if (Profile::exists(profileName))
        {
            qDebug() << "Setting profile to" << profileName;
            if (Profile::isEncrypted(profileName))
            {
                Settings::getInstance().setCurrentProfile(profileName);
            }
            else
            {
                Profile* profile = Profile::loadProfile(profileName);
                if (!profile)
                {
                    qCritical() << "-p profile" << profileName + ".tox" << " couldn't be loaded";
                    return EXIT_FAILURE;
                }
                Nexus::getInstance().setProfile(profile);
            }
        }
        else
        {
            qCritical() << "-p profile" << profileName + ".tox" << "doesn't exist";
            return EXIT_FAILURE;
        }
    }

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
        time_t event = ipc.postEvent("activate");
        if (!ipc.waitUntilAccepted(event, 2))
        {
            return EXIT_SUCCESS;
        }
    }
#endif

    // Autologin
    if (Settings::getInstance().getAutoLogin())
    {
        QString profileName = Settings::getInstance().getCurrentProfile();
        if (Profile::exists(profileName) && !Profile::isEncrypted(profileName))
        {
            Profile* profile = Profile::loadProfile(profileName);
            if (profile)
                Nexus::getInstance().setProfile(profile);
        }
    }

    Nexus::getInstance().start();

    // Run
    int errorcode = a.exec();

#ifdef LOG_TO_FILE
    logFileStream.release();
#endif

    Nexus::destroyInstance();
    CameraSource::destroyInstance();
    Settings::destroyInstance();
    qDebug() << "Clean exit with status"<<errorcode;
    return errorcode;
}
