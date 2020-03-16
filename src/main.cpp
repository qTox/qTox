/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#include "src/audio/audio.h"
#include "src/ipc.h"
#include "src/net/toxuri.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/persistence/toxsave.h"
#include "src/video/camerasource.h"
#include "src/widget/loginscreen.h"
#include "src/widget/translator.h"
#include "widget/widget.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QMutex>
#include <QMutexLocker>

#include <QtWidgets/QMessageBox>
#include <ctime>
#include <sodium.h>
#include <stdio.h>

#if defined(Q_OS_OSX)
#include "platform/install_osx.h"
#endif

#if defined(Q_OS_UNIX)
#include "platform/posixsignalnotifier.h"
#endif

#ifdef LOG_TO_FILE
static QAtomicPointer<FILE> logFileFile = nullptr;
static QList<QByteArray>* logBuffer =
    new QList<QByteArray>(); // Store log messages until log file opened
QMutex* logBufferMutex = new QMutex();
#endif

void cleanup()
{
    // force save early even though destruction saves, because Windows OS will
    // close qTox before cleanup() is finished if logging out or shutting down,
    // once the top level window has exited, which occurs in ~Widget within
    // ~Nexus. Re-ordering Nexus destruction is not trivial.
    auto& s = Settings::getInstance();
    s.saveGlobal();
    s.savePersonal();
    s.sync();

    Nexus::destroyInstance();
    CameraSource::destroyInstance();
    Settings::destroyInstance();
    qDebug() << "Cleanup success";

#ifdef LOG_TO_FILE
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    FILE* f = logFileFile.loadRelaxed();
#else
    FILE* f = logFileFile.load();
#endif
    if (f != nullptr) {
        fclose(f);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        logFileFile.storeRelaxed(nullptr); // atomically disable logging to file
#else
        logFileFile.store(nullptr); // atomically disable logging to file
#endif
    }
#endif
}

void logMessageHandler(QtMsgType type, const QMessageLogContext& ctxt, const QString& msg)
{
    // Silence qWarning spam due to bug in QTextBrowser (trying to open a file for base64 images)
    if (ctxt.function == QString("virtual bool QFSFileEngine::open(QIODevice::OpenMode)")
        && msg == QString("QFSFileEngine::open: No file name specified"))
        return;

    QString file = ctxt.file;
    // We're not using QT_MESSAGELOG_FILE here, because that can be 0, NULL, or
    // nullptr in release builds.
    QString path = QString(__FILE__);
    path = path.left(path.lastIndexOf('/') + 1);
    if (file.startsWith(path)) {
        file = file.mid(path.length());
    }

    // Time should be in UTC to save user privacy on log sharing
    QTime time = QDateTime::currentDateTime().toUTC().time();
    QString LogMsg =
        QString("[%1 UTC] %2:%3 : ").arg(time.toString("HH:mm:ss.zzz")).arg(file).arg(ctxt.line);
    switch (type) {
    case QtDebugMsg:
        LogMsg += "Debug";
        break;
    case QtInfoMsg:
        LogMsg += "Info";
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
    QByteArray LogMsgBytes = LogMsg.toUtf8();
    fwrite(LogMsgBytes.constData(), 1, LogMsgBytes.size(), stderr);

#ifdef LOG_TO_FILE
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    FILE* logFilePtr = logFileFile.loadRelaxed(); // atomically load the file pointer
#else
    FILE* logFilePtr = logFileFile.load(); // atomically load the file pointer
#endif
    if (!logFilePtr) {
        logBufferMutex->lock();
        if (logBuffer)
            logBuffer->append(LogMsgBytes);

        logBufferMutex->unlock();
    } else {
        logBufferMutex->lock();
        if (logBuffer) {
            // empty logBuffer to file
            foreach (QByteArray msg, *logBuffer)
                fwrite(msg.constData(), 1, msg.size(), logFilePtr);

            delete logBuffer; // no longer needed
            logBuffer = nullptr;
        }
        logBufferMutex->unlock();

        fwrite(LogMsgBytes.constData(), 1, LogMsgBytes.size(), logFilePtr);
        fflush(logFilePtr);
    }
#endif
}

int main(int argc, char* argv[])
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    qInstallMessageHandler(logMessageHandler);

    // initialize random number generator
#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
    qsrand(time(nullptr));
#endif

    std::unique_ptr<QApplication> a(new QApplication(argc, argv));

#if defined(Q_OS_UNIX)
    // PosixSignalNotifier is used only for terminating signals,
    // so it's connected directly to quit() without any filtering.
    QObject::connect(&PosixSignalNotifier::globalInstance(), &PosixSignalNotifier::activated,
                     a.get(), &QApplication::quit);
    PosixSignalNotifier::watchCommonTerminatingSignals();
#endif

    a->setApplicationName("qTox");
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    a->setDesktopFileName("io.github.qtox.qTox");
#endif
    a->setApplicationVersion("\nGit commit: " + QString(GIT_VERSION));

    // Install Unicode 6.1 supporting font
    // Keep this as close to the beginning of `main()` as possible, otherwise
    // on systems that have poor support for Unicode qTox will look bad.
    if (QFontDatabase::addApplicationFont("://font/DejaVuSans.ttf") == -1) {
        qWarning() << "Couldn't load font";
    }


#if defined(Q_OS_OSX)
    // TODO: Add setting to enable this feature.
    // osx::moveToAppFolder();
    osx::migrateProfiles();
#endif

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
    qsrand(time(nullptr));
#endif
    Settings& settings = Settings::getInstance();
    QString locale = settings.getTranslation();
    Translator::translate(locale);

    // Process arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("qTox, version: " + QString(GIT_VERSION));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("uri", QObject::tr("Tox URI to parse"));
    parser.addOption(
        QCommandLineOption(QStringList() << "p"
                                         << "profile",
                           QObject::tr("Starts new instance and loads specified profile."),
                           QObject::tr("profile")));
    parser.addOption(
        QCommandLineOption(QStringList() << "l"
                                         << "login",
                           QObject::tr("Starts new instance and opens the login screen.")));
    parser.addOption(QCommandLineOption(QStringList() << "I"
                                                      << "IPv6",
                                        QObject::tr("Sets IPv6 <on>/<off>. Default is ON."),
                                        QObject::tr("on/off")));
    parser.addOption(QCommandLineOption(QStringList() << "U"
                                                      << "UDP",
                                        QObject::tr("Sets UDP <on>/<off>. Default is ON."),
                                        QObject::tr("on/off")));
    parser.addOption(
        QCommandLineOption(QStringList() << "L"
                                         << "LAN",
                           QObject::tr(
                               "Sets LAN discovery <on>/<off>. UDP off overrides. Default is ON."),
                           QObject::tr("on/off")));
    parser.addOption(QCommandLineOption(QStringList() << "P"
                                                      << "proxy",
                                        QObject::tr("Sets proxy settings. Default is NONE."),
                                        QObject::tr("(SOCKS5/HTTP/NONE):(ADDRESS):(PORT)")));
    parser.process(*a);

    uint32_t profileId = settings.getCurrentProfileId();
    IPC ipc(profileId);
    if (ipc.isAttached()) {
        QObject::connect(&settings, &Settings::currentProfileIdChanged, &ipc, &IPC::setProfileId);
    } else {
        qWarning() << "Can't init IPC, maybe we're in a jail? Continuing with reduced multi-client functionality.";
    }

    // For the auto-updater
    if (sodium_init() < 0) {
        qCritical() << "Can't init libsodium";
        return EXIT_FAILURE;
    }

#ifdef LOG_TO_FILE
    QString logFileDir = settings.getAppCacheDirPath();
    QDir(logFileDir).mkpath(".");

    QString logfile = logFileDir + "qtox.log";
    FILE* mainLogFilePtr = fopen(logfile.toLocal8Bit().constData(), "a");

    // Trim log file if over 1MB
    if (QFileInfo(logfile).size() > 1000000) {
        qDebug() << "Log file over 1MB, rotating...";

        // close old logfile (need for windows)
        if (mainLogFilePtr)
            fclose(mainLogFilePtr);

        QDir dir(logFileDir);

        // Check if log.1 already exists, and if so, delete it
        if (dir.remove(logFileDir + "qtox.log.1"))
            qDebug() << "Removed old log successfully";
        else
            qWarning() << "Unable to remove old log file";

        if (!dir.rename(logFileDir + "qtox.log", logFileDir + "qtox.log.1"))
            qCritical() << "Unable to move logs";

        // open a new logfile
        mainLogFilePtr = fopen(logfile.toLocal8Bit().constData(), "a");
    }

    if (!mainLogFilePtr)
        qCritical() << "Couldn't open logfile" << logfile;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    logFileFile.storeRelaxed(mainLogFilePtr); // atomically set the logFile
#else
    logFileFile.store(mainLogFilePtr); // atomically set the logFile
#endif
#endif

    // Windows platform plugins DLL hell fix
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());
    a->addLibraryPath("platforms");

    qDebug() << "commit: " << GIT_VERSION;

    QString profileName;
    bool autoLogin = settings.getAutoLogin();

    uint32_t ipcDest = 0;
    bool doIpc = ipc.isAttached();
    QString eventType, firstParam;
    if (parser.isSet("p")) {
        profileName = parser.value("p");
        if (!Profile::exists(profileName)) {
            qWarning() << "-p profile" << profileName + ".tox"
                       << "doesn't exist, opening login screen";
            doIpc = false;
            autoLogin = false;
        } else {
            ipcDest = Settings::makeProfileId(profileName);
            autoLogin = true;
        }
    } else if (parser.isSet("l")) {
        doIpc = false;
        autoLogin = false;
    } else {
        profileName = settings.getCurrentProfile();
    }

    if (parser.positionalArguments().empty()) {
        eventType = "activate";
    } else {
        firstParam = parser.positionalArguments()[0];
        // Tox URIs. If there's already another qTox instance running, we ask it to handle the URI
        // and we exit
        // Otherwise we start a new qTox instance and process it ourselves
        if (firstParam.startsWith("tox:")) {
            eventType = "uri";
        } else if (firstParam.endsWith(".tox")) {
            eventType = "save";
        } else {
            qCritical() << "Invalid argument";
            return EXIT_FAILURE;
        }
    }

    if (doIpc && !ipc.isCurrentOwner()) {
        time_t event = ipc.postEvent(eventType, firstParam.toUtf8(), ipcDest);
        // If someone else processed it, we're done here, no need to actually start qTox
        if (ipc.waitUntilAccepted(event, 2)) {
            if (eventType == "activate") {
                qDebug()
                    << "Another qTox instance is already running. If you want to start a second "
                       "instance, please open login screen (qtox -l) or start with a profile (qtox "
                       "-p <profile name>).";
            } else {
                qDebug() << "Event" << eventType << "was handled by other client.";
            }
            return EXIT_SUCCESS;
        }
    }

    if (!Settings::verifyProxySettings(parser)) {
        return -1;
    }

    // TODO(sudden6): remove once we get rid of Nexus
    Nexus& nexus = Nexus::getInstance();
    // TODO(kriby): Consider moving application initializing variables into a globalSettings object
    //  note: Because Settings is shouldering global settings as well as model specific ones it
    //  cannot be integrated into a central model object yet
    nexus.setSettings(&settings);

    // Autologin
    // TODO (kriby): Shift responsibility of linking views to model objects from nexus
    // Further: generate view instances separately (loginScreen, mainGUI, audio)
    Profile* profile = nullptr;
    if (autoLogin && Profile::exists(profileName) && !Profile::isEncrypted(profileName)) {
        profile = Profile::loadProfile(profileName, QString(), settings, &parser);
        if (!profile) {
            QMessageBox::information(nullptr, QObject::tr("Error"),
                                     QObject::tr("Failed to load profile automatically."));
        }
    }
    if (profile) {
        nexus.bootstrapWithProfile(profile);
    } else {
        nexus.setParser(&parser);
        int returnval = nexus.showLogin(profileName);
        if (returnval == QDialog::Rejected) {
            return -1;
        }
    }

    if (ipc.isAttached()) {
        // Start to accept Inter-process communication
        ipc.registerEventHandler("uri", &toxURIEventHandler);
        ipc.registerEventHandler("save", &toxSaveEventHandler);
        ipc.registerEventHandler("activate", &toxActivateEventHandler);
    }

    // Event was not handled by already running instance therefore we handle it ourselves
    if (eventType == "uri")
        handleToxURI(firstParam.toUtf8());
    else if (eventType == "save")
        handleToxSave(firstParam.toUtf8());

    QObject::connect(a.get(), &QApplication::aboutToQuit, cleanup);

    // Run
    int errorcode = a->exec();

    qDebug() << "Exit with status" << errorcode;
    return errorcode;
}
