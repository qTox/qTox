/*
    Copyright © 2022 by The qTox Project Contributors

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

#include "appmanager.h"

#include "src/widget/tool/messageboxmanager.h"
#include "src/persistence/settings.h"
#include "src/persistence/toxsave.h"
#include "src/persistence/profile.h"
#include "src/ipc.h"
#include "src/widget/translator.h"
#include "src/nexus.h"
#include "src/net/toxuri.h"
#include "src/widget/widget.h"
#include "src/video/camerasource.h"

#if defined(Q_OS_UNIX)
#include "src/platform/posixsignalnotifier.h"
#endif

#include <QApplication>
#include <QFontDatabase>
#include <QCommandLineParser>
#include <QDir>
#include <QMessageBox>
#include <QObject>

namespace
{
// logMessageHandler and associated data must be static due to qInstallMessageHandler's
// inability to register a void* to get back to a class
#ifdef LOG_TO_FILE
QAtomicPointer<FILE> logFileFile = nullptr;
QList<QByteArray>* logBuffer =
    new QList<QByteArray>(); // Store log messages until log file opened
QMutex* logBufferMutex = new QMutex();
#endif

void logMessageHandler(QtMsgType type, const QMessageLogContext& ctxt, const QString& msg)
{
    // Silence qWarning spam due to bug in QTextBrowser (trying to open a file for base64 images)
    if (QString::fromUtf8(ctxt.function) == QString("virtual bool QFSFileEngine::open(QIODevice::OpenMode)")
        && msg == QString("QFSFileEngine::open: No file name specified")) {
        return;
    }
    if (msg.startsWith("Unable to find any suggestion for")) {
        // Prevent sonnet's complaints from leaking user chat messages to logs
        return;
    }

    if (msg == QString("attempted to send message with network family 10 (probably IPv6) on IPv4 socket")) {
        // non-stop c-toxcore spam for IPv4 users: https://github.com/TokTok/c-toxcore/issues/1432
        return;
    }

    QRegExp snoreFilter{QStringLiteral("Snore::Notification.*was already closed")};
    if (type == QtWarningMsg
        && msg.contains(snoreFilter))
    {
        // snorenotify logs this when we call requestCloseNotification correctly. The behaviour still works, so we'll
        // just mask the warning for now. The issue has been reported upstream:
        // https://github.com/qTox/qTox/pull/6073#pullrequestreview-420748519
        return;
    }

    QString file = QString::fromUtf8(ctxt.file);
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
            foreach (QByteArray bufferedMsg, *logBuffer)
                fwrite(bufferedMsg.constData(), 1, bufferedMsg.size(), logFilePtr);

            delete logBuffer; // no longer needed
            logBuffer = nullptr;
        }
        logBufferMutex->unlock();

        fwrite(LogMsgBytes.constData(), 1, LogMsgBytes.size(), logFilePtr);
        fflush(logFilePtr);
    }
#endif
}

bool toxURIEventHandler(const QByteArray& eventData, void* userData)
{
    ToxURIDialog* uriDialog = static_cast<ToxURIDialog*>(userData);
    if (!eventData.startsWith("tox:")) {
        return false;
    }

    if (!uriDialog) {
        return false;
    }

    uriDialog->handleToxURI(QString::fromUtf8(eventData));
    return true;
}
} // namespace

AppManager::AppManager(int& argc, char** argv)
    : qapp((preConstructionInitialization(), new QApplication(argc, argv)))
    , messageBoxManager(new MessageBoxManager(nullptr))
    , settings(new Settings(*messageBoxManager))
    , ipc(new IPC(settings->getCurrentProfileId()))
{
}

void AppManager::preConstructionInitialization()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    qInstallMessageHandler(logMessageHandler);
}

int AppManager::run()
{
#if defined(Q_OS_UNIX)
    // PosixSignalNotifier is used only for terminating signals,
    // so it's connected directly to quit() without any filtering.
    connect(&PosixSignalNotifier::globalInstance(), &PosixSignalNotifier::activated,
                     qapp.get(), &QApplication::quit);
    PosixSignalNotifier::watchCommonTerminatingSignals();
#endif

    qapp->setApplicationName("qTox");
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    qapp->setDesktopFileName("io.github.qtox.qTox");
#endif
    qapp->setApplicationVersion("\nGit commit: " + QString(GIT_VERSION));

    // Install Unicode 6.1 supporting font
    // Keep this as close to the beginning of `main()` as possible, otherwise
    // on systems that have poor support for Unicode qTox will look bad.
    if (QFontDatabase::addApplicationFont("://font/DejaVuSans.ttf") == -1) {
        qWarning() << "Couldn't load font";
    }

    QString locale = settings->getTranslation();
    // We need to init the resources in the translations_library explicitely.
    // See https://doc.qt.io/qt-5/resources.html#using-resources-in-a-library
    Q_INIT_RESOURCE(translations);
    Translator::translate(locale);

    // Process arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("qTox, version: " + QString(GIT_VERSION));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("uri", tr("Tox URI to parse"));
    parser.addOption(
        QCommandLineOption(QStringList() << "p"
                                         << "profile",
                           tr("Starts new instance and loads specified profile."),
                           tr("profile")));
    parser.addOption(
        QCommandLineOption(QStringList() << "l"
                                         << "login",
                           tr("Starts new instance and opens the login screen.")));
    parser.addOption(QCommandLineOption(QStringList() << "I"
                                                      << "IPv6",
                                        tr("Sets IPv6 <on>/<off>. Default is ON."),
                                        tr("on/off")));
    parser.addOption(QCommandLineOption(QStringList() << "U"
                                                      << "UDP",
                                        tr("Sets UDP <on>/<off>. Default is ON."),
                                        tr("on/off")));
    parser.addOption(
        QCommandLineOption(QStringList() << "L"
                                         << "LAN",
                           tr(
                               "Sets LAN discovery <on>/<off>. UDP off overrides. Default is ON."),
                           tr("on/off")));
    parser.addOption(QCommandLineOption(QStringList() << "P"
                                                      << "proxy",
                                        tr("Sets proxy settings. Default is NONE."),
                                        tr("(SOCKS5/HTTP/NONE):(ADDRESS):(PORT)")));
    parser.process(*qapp);

    if (ipc->isAttached()) {
        connect(settings.get(), &Settings::currentProfileIdChanged, ipc.get(), &IPC::setProfileId);
    } else {
        qWarning() << "Can't init IPC, maybe we're in a jail? Continuing with reduced multi-client functionality.";
    }

#ifdef LOG_TO_FILE
    QString logFileDir = settings->getPaths().getAppCacheDirPath();
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
    qapp->addLibraryPath("platforms");

    qDebug() << "commit: " << GIT_VERSION;

    QString profileName;
    bool autoLogin = settings->getAutoLogin();

    uint32_t ipcDest = 0;
    bool doIpc = ipc->isAttached();
    QString eventType, firstParam;
    if (parser.isSet("p")) {
        profileName = parser.value("p");
        if (!Profile::exists(profileName, settings->getPaths())) {
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
        profileName = settings->getCurrentProfile();
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
            eventType = ToxSave::eventHandlerKey;
        } else {
            qCritical() << "Invalid argument";
            return EXIT_FAILURE;
        }
    }

    if (doIpc && !ipc->isCurrentOwner()) {
        time_t event = ipc->postEvent(eventType, firstParam.toUtf8(), ipcDest);
        // If someone else processed it, we're done here, no need to actually start qTox
        if (ipc->waitUntilAccepted(event, 2)) {
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

    // TODO(kriby): Consider moving application initializing variables into a globalSettings object
    //  note: Because Settings is shouldering global settings as well as model specific ones it
    //  cannot be integrated into a central model object yet
    cameraSource = std::unique_ptr<CameraSource>(new CameraSource{*settings});
    nexus = std::unique_ptr<Nexus>(new Nexus{*settings, *messageBoxManager, *cameraSource, *ipc});
    // Autologin
    // TODO (kriby): Shift responsibility of linking views to model objects from nexus
    // Further: generate view instances separately (loginScreen, mainGUI, audio)
    Profile* profile = nullptr;
    if (autoLogin && Profile::exists(profileName, settings->getPaths()) && !Profile::isEncrypted(profileName, settings->getPaths())) {
        profile = Profile::loadProfile(profileName, QString(), *settings, &parser, *cameraSource, *messageBoxManager);
        if (!profile) {
            QMessageBox::information(nullptr, tr("Error"),
                                     tr("Failed to load profile automatically."));
        }
    }
    if (profile) {
        nexus->bootstrapWithProfile(profile);
    } else {
        nexus->setParser(&parser);
        int returnval = nexus->showLogin(profileName);
        if (returnval == QDialog::Rejected) {
            return -1;
        }
        profile = nexus->getProfile();
    }

    uriDialog = std::unique_ptr<ToxURIDialog>(new ToxURIDialog(nullptr, profile->getCore(), *messageBoxManager));

    if (ipc->isAttached()) {
        // Start to accept Inter-process communication
        ipc->registerEventHandler("uri", &toxURIEventHandler, uriDialog.get());
        nexus->registerIpcHandlers();
    }

    // Event was not handled by already running instance therefore we handle it ourselves
    if (eventType == "uri") {
        uriDialog->handleToxURI(firstParam);
    } else if (eventType == ToxSave::eventHandlerKey) {
        nexus->handleToxSave(firstParam);
    }

    connect(qapp.get(), &QApplication::aboutToQuit, this, &AppManager::cleanup);

    return qapp->exec();
}

AppManager::~AppManager() = default;

void AppManager::cleanup()
{
    // force save early even though destruction saves, because Windows OS will
    // close qTox before cleanup() is finished if logging out or shutting down,
    // once the top level window has exited, which occurs in ~Widget within
    // ~nexus-> Re-ordering Nexus destruction is not trivial.
    if (settings) {
        settings->saveGlobal();
        settings->savePersonal();
        settings->sync();
    }

    nexus.reset();
    settings.reset();
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
