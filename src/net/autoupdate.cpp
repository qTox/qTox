/*
    Copyright © 2014-2015 by The qTox Project Contributors

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

#include "src/net/autoupdate.h"
#include "src/persistence/serialize.h"
#include "src/persistence/settings.h"
#include "src/widget/widget.h"
#include "src/widget/gui.h"
#include "src/nexus.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QtConcurrent/QtConcurrent>
#include <QMessageBox>
#include <QMutexLocker>
#include <iostream>

#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>
#endif

/**
 * @file autoupdate.cpp
 *
 * For now we only support auto updates on Windows and OS X, although extending it is not a technical issue.
 * Linux users are expected to use their package managers or update manually through official channels.
 */

#ifdef Q_OS_WIN
#ifdef Q_OS_WIN64
const QString AutoUpdater::platform = "win64";
#else
const QString AutoUpdater::platform = "win32";
#endif
const QString AutoUpdater::updaterBin = "qtox-updater.exe";
const QString AutoUpdater::updateServer = "https://qtox-win.pkg.tox.chat";

unsigned char AutoUpdater::key[crypto_sign_PUBLICKEYBYTES] =
{
    0x20, 0x89, 0x39, 0xaa, 0x9a, 0xe8, 0xb5, 0x21, 0x0e, 0xac, 0x02, 0xa9, 0xc4, 0x92, 0xd9, 0xa2,
    0x17, 0x83, 0xbd, 0x78, 0x0a, 0xda, 0x33, 0xcd, 0xa5, 0xc6, 0x44, 0xc7, 0xfc, 0xed, 0x00, 0x13
};

#elif defined(Q_OS_OSX)
const QString AutoUpdater::platform = "osx";
const QString AutoUpdater::updaterBin = "/Applications/qtox.app/Contents/MacOS/updater";
const QString AutoUpdater::updateServer = "https://dist-build.tox.im";

unsigned char AutoUpdater::key[crypto_sign_PUBLICKEYBYTES] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#else
const QString AutoUpdater::platform;
const QString AutoUpdater::updaterBin;
const QString AutoUpdater::updateServer;
unsigned char AutoUpdater::key[crypto_sign_PUBLICKEYBYTES];
#endif

/**
 * @var unsigned char AutoUpdater::UpdateFileMeta::sig[crypto_sign_BYTES]
 * @brief Signature of the file (ed25519)
 *
 * @var QString AutoUpdater::UpdateFileMeta::id
 * @brief Unique id of the file
 *
 * @var QString AutoUpdater::UpdateFileMeta::installpath
 * @brief Local path including the file name. May be relative to qtox-updater or absolute
 *
 * @var uint64_t AutoUpdater::UpdateFileMeta::size
 * @brief Size in bytes of the file
 */

/**
 * @var static const QString AutoUpdater::updateServer
 * @brief Hostname of the qTox update server
 *
 * @var static const QString AutoUpdater::platform
 * @brief Name of platform we're trying to get updates for
 *
 * @var static const QString AutoUpdater::checkURI
 * @brief URI of the file containing the latest version string
 *
 * @var static const QString AutoUpdater::flistURI
 * @brief URI of the file containing info on each file (hash, signature, size, name, ..)
 *
 * @var static const QString AutoUpdater::filesURI
 * @brief URI of the actual files of the latest version
 *
 * @var static const QString AutoUpdater::updaterBin
 * @brief Path to the qtox-updater binary
 *
 * @var static std::atomic_bool AutoUpdater::abortFlag
 * @brief If true, try to abort everything.
 *
 * @var static std::atomic_bool AutoUpdater::isDownloadingUpdate
 * @brief We'll pretend there's no new update available if we're already updating
 *
 * @var static QMutex AutoUpdater::progressVersionMutex
 * @brief No, we can't just make the QString atomic
 */

const QString AutoUpdater::checkURI = AutoUpdater::updateServer+"/qtox/"+AutoUpdater::platform+"/version";
const QString AutoUpdater::flistURI = AutoUpdater::updateServer+"/qtox/"+AutoUpdater::platform+"/flist";
const QString AutoUpdater::filesURI = AutoUpdater::updateServer+"/qtox/"+AutoUpdater::platform+"/files/";
std::atomic_bool AutoUpdater::abortFlag{false};
std::atomic_bool AutoUpdater::isDownloadingUpdate{false};
std::atomic<float> AutoUpdater::progressValue{0};
QString AutoUpdater::progressVersion;
QMutex AutoUpdater::progressVersionMutex;

/**
 * @class AutoUpdater
 * @brief Handles checking and applying updates for qTox.
 * @note Do *NOT* use auto update unless AUTOUPDATE_ENABLED is defined to 1.
 */

/**
 * @brief Checks if an update is available for download.
 * @return True if an update is available for download, false otherwise.
 *
 * Connects to the qTox update server, and check if an update is available for download
 * Will call getUpdateVersion, and as such may block and processEvents.
 */
bool AutoUpdater::isUpdateAvailable()
{
    if (isDownloadingUpdate)
        return false;

    QString updaterPath = updaterBin.startsWith('/') ? updaterBin : qApp->applicationDirPath()+'/'+updaterBin;
    if (!QFile::exists(updaterPath))
        return false;

    QByteArray updateFlist = getUpdateFlist();
    QList<UpdateFileMeta> diff = genUpdateDiff(parseFlist(updateFlist));
    return !diff.isEmpty();
}

/**
 * @brief Fetch the version info of the last update available from the qTox update server
 * @note Will try to follow qTox's proxy settings, may block and processEvents
 * @return Avaliable version info.
 */
AutoUpdater::VersionInfo AutoUpdater::getUpdateVersion()
{
    VersionInfo versionInfo;
    versionInfo.timestamp = 0;

    // Updates only for supported platforms
    if (platform.isEmpty())
        return versionInfo;

    if (abortFlag)
        return versionInfo;

    QNetworkAccessManager *manager = new QNetworkAccessManager;
    manager->setProxy(Settings::getInstance().getProxy());
    QNetworkReply* reply = manager->get(QNetworkRequest(QUrl(checkURI)));
    while (!reply->isFinished())
    {
        if (abortFlag)
            return versionInfo;
        qApp->processEvents();
    }

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "getUpdateVersion: network error: " + reply->errorString();
        reply->deleteLater();
        manager->deleteLater();
        return versionInfo;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();
    manager->deleteLater();
    if (data.size() < (int)(1+crypto_sign_BYTES))
        return versionInfo;

    // Check updater protocol version
    if ((int)data[0] != '3')
    {
        qWarning() << "getUpdateVersion: Bad version " << (uint8_t)data[0];
        return versionInfo;
    }

    // Check the signature
    QByteArray sigData = data.mid(1, crypto_sign_BYTES);
    unsigned char* sig = (unsigned char*)sigData.data();
    QByteArray msgData = data.mid(1+crypto_sign_BYTES);
    unsigned char* msg = (unsigned char*)msgData.data();

    if (crypto_sign_verify_detached(sig, msg, msgData.size(), key) != 0)
    {
        qCritical() << "getUpdateVersion: RECEIVED FORGED VERSION FILE FROM "<<updateServer;
        return versionInfo;
    }

    int sepPos = msgData.indexOf('!');
    versionInfo.timestamp = QString(msgData.left(sepPos)).toInt();
    versionInfo.versionString = msgData.mid(sepPos+1);

    qDebug() << "timestamp:"<<versionInfo.timestamp << ", str:"<<versionInfo.versionString;

    return versionInfo;
}

/**
 * @brief Parses and validates a flist file.
 * @param flistData Install file data.
 * @return An empty list on error.
 */
QList<AutoUpdater::UpdateFileMeta> AutoUpdater::parseFlist(QByteArray flistData)
{
    QList<UpdateFileMeta> flist;

    if (flistData.isEmpty())
    {
        qWarning() << "parseflist: Empty data";
        return flist;
    }

    // Check version
    if (flistData[0] != '1')
    {
        qWarning() << "parseflist: Bad version "<<(uint8_t)flistData[0];
        return flist;
    }
    flistData = flistData.mid(1);

    // Check signature
    if (flistData.size() < (int)(crypto_sign_BYTES))
    {
        qWarning() << "parseflist: Truncated data";
        return flist;
    }
    else
    {
        QByteArray msgData = flistData.mid(crypto_sign_BYTES);
        unsigned char* msg = (unsigned char*)msgData.data();
        if (crypto_sign_verify_detached((unsigned char*)flistData.data(), msg, msgData.size(), key) != 0)
        {
            qCritical() << "parseflist: FORGED FLIST FILE";
            return flist;
        }
        flistData = flistData.mid(crypto_sign_BYTES);
    }

    // Parse. We assume no errors handling needed since the signature is valid.
    while (!flistData.isEmpty())
    {
        UpdateFileMeta newFile;

        memcpy(newFile.sig, flistData.data(), crypto_sign_BYTES);
        flistData = flistData.mid(crypto_sign_BYTES);

        newFile.id = dataToString(flistData);
        flistData = flistData.mid(newFile.id.size() + getVUint32Size(flistData));

        newFile.installpath = dataToString(flistData);
        flistData = flistData.mid(newFile.installpath.size() + getVUint32Size(flistData));

        newFile.size = dataToUint64(flistData);
        flistData = flistData.mid(8);

        flist += newFile;
    }

    return flist;
}

/**
 * @brief Gets the update server's flist.
 * @note Will try to follow qTox's proxy settings, may block and processEvents
 * @return An empty array on error
 */
QByteArray AutoUpdater::getUpdateFlist()
{
    QByteArray flist;

    QNetworkAccessManager *manager = new QNetworkAccessManager;
    manager->setProxy(Settings::getInstance().getProxy());
    QNetworkReply* reply = manager->get(QNetworkRequest(QUrl(flistURI)));
    while (!reply->isFinished())
    {
        if (abortFlag)
            return flist;
        qApp->processEvents();
    }

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "getUpdateFlist: network error: " + reply->errorString();
        reply->deleteLater();
        manager->deleteLater();
        return flist;
    }

    flist = reply->readAll();
    reply->deleteLater();
    manager->deleteLater();

    return flist;
}

/**
 * @brief Generates a list of files we need to update.
 * @param updateFlist List of files available to update.
 * @return List of files we need to update.
 */
QList<AutoUpdater::UpdateFileMeta> AutoUpdater::genUpdateDiff(QList<UpdateFileMeta> updateFlist)
{
    QList<UpdateFileMeta> diff;

    for (UpdateFileMeta file : updateFlist)
        if (!isUpToDate(file))
            diff += file;

    return diff;
}

/**
 * @brief Checks if we have an up to date version of this file locally installed.
 * @param fileMeta File to check.
 * @return True if file doesn't need updates, false if need.
 */
bool AutoUpdater::isUpToDate(AutoUpdater::UpdateFileMeta fileMeta)
{
    QString appDir = qApp->applicationDirPath();
    QFile file(appDir+QDir::separator()+fileMeta.installpath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    // If the data we have is corrupted or old, mark it for update
    QByteArray data = file.readAll();
    if (crypto_sign_verify_detached(fileMeta.sig, (unsigned char*)data.data(), data.size(), key) != 0)
        return false;

    return true;
}

/**
 * @brief Tries to fetch the file from the update server.
 * @note Note that a file with an empty but non-null QByteArray is not an error, merely a file of size 0.
 * @note Will try to follow qTox's proxy settings, may block and processEvents.
 * @param fileMeta Meta data fo file to update.
 * @param progressCallback Callback function, which will connected with QNetworkReply::downloadProgress
 * @return A file with a null QByteArray on error.
 */
AutoUpdater::UpdateFile AutoUpdater::getUpdateFile(UpdateFileMeta fileMeta,
                                        std::function<void(int,int)> progressCallback)
{
    UpdateFile file;
    file.metadata = fileMeta;

    QNetworkAccessManager *manager = new QNetworkAccessManager;
    manager->setProxy(Settings::getInstance().getProxy());
    QNetworkReply* reply = manager->get(QNetworkRequest(QUrl(filesURI+fileMeta.id)));
    QObject::connect(reply, &QNetworkReply::downloadProgress, progressCallback);
    while (!reply->isFinished())
    {
        if (abortFlag)
            return file;
        qApp->processEvents();
    }

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "getUpdateFile: network error: " + reply->errorString();
        reply->deleteLater();
        manager->deleteLater();
        return file;
    }

    file.data = reply->readAll();
    reply->deleteLater();
    manager->deleteLater();

    return file;
}

/**
 * @brief Will try to download an update, if successful qTox will apply it after a restart
 * @note Will try to follow qTox's proxy settings, may block and processEvents
 * @result True if successful and qTox will apply it after a restart
 */
bool AutoUpdater::downloadUpdate()
{
    // Updates only for supported platforms
    if (platform.isEmpty())
        return false;

    bool expectFalse = false;
    if (!isDownloadingUpdate.compare_exchange_strong(expectFalse,true))
        return false;

    // Get a list of files to update
    QByteArray newFlistData = getUpdateFlist();
    QList<UpdateFileMeta> newFlist = parseFlist(newFlistData);
    QList<UpdateFileMeta> diff = genUpdateDiff(newFlist);

    // Progress
    progressValue = 0;

    if (abortFlag)
    {
        isDownloadingUpdate = false;
        return false;
    }

    qDebug() << "Need to update" << diff.size() << "files";

    // Create an empty directory to download updates into
    QString updateDirStr = Settings::getInstance().getSettingsDirPath() + "/update/";
    QDir updateDir(updateDirStr);
    if (!updateDir.exists())
        QDir().mkdir(updateDirStr);
    updateDir = QDir(updateDirStr);
    if (!updateDir.exists())
    {
        qWarning() << "downloadUpdate: Can't create update directory, aborting...";
        isDownloadingUpdate = false;
        return false;
    }

    // Write the new flist for the updater
    QFile newFlistFile(updateDirStr+"flist");
    if (!newFlistFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qWarning() << "downloadUpdate: Can't save new flist file, aborting...";
        isDownloadingUpdate = false;
        return false;
    }
    newFlistFile.write(newFlistData);
    newFlistFile.close();

    progressValue = 1;

    // Download and write each new file
    for (UpdateFileMeta fileMeta : diff)
    {
        float initialProgress = progressValue, step = 99./diff.size();
        auto stepProgressCallback = [&](int current, int total)
        {
            progressValue = initialProgress + step * (float)current/total;
        };

        if (abortFlag)
        {
            isDownloadingUpdate = false;
            return false;
        }

        // Skip files we already have
        QFile fileFile(updateDirStr+fileMeta.installpath);
        if (fileFile.open(QIODevice::ReadOnly) && fileFile.size() == (qint64)fileMeta.size)
        {
            qDebug() << "Skipping already downloaded file   '" + fileMeta.installpath+ "'";
            fileFile.close();
            progressValue = initialProgress + step;
            continue;
        }

        qDebug() << "Downloading '" + fileMeta.installpath + "' ...";

        // Create subdirs if necessary
        QString fileDirStr{QFileInfo(updateDirStr+fileMeta.installpath).absolutePath()};
        if (!QDir(fileDirStr).exists())
            QDir().mkpath(fileDirStr);

        // Download
        UpdateFile file = getUpdateFile(fileMeta, stepProgressCallback);
        if (abortFlag)
            goto fail;
        if (file.data.isNull())
        {
            qCritical() << "downloadUpdate: Error downloading a file, aborting...";
            goto fail;
        }

        // Check signature
        if (crypto_sign_verify_detached(file.metadata.sig, (unsigned char*)file.data.data(),
                                        file.data.size(), key) != 0)
        {
            qCritical() << "downloadUpdate: RECEIVED FORGED FILE, aborting...";
            goto fail;
        }

        // Save
        if (!fileFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            qCritical() << "downloadUpdate: Can't save new update file, aborting...";
            goto fail;
        }
        fileFile.write(file.data);
        fileFile.close();

        progressValue = initialProgress + step;
    }

    qDebug() << "downloadUpdate: The update is ready, it'll be installed on the next restart";

    isDownloadingUpdate = false;
    progressValue = 100;
    return true;

fail:
    isDownloadingUpdate = false;
    progressValue = 0;
    setProgressVersion("");
    return false;
}

/**
 * @brief Checks if an update is downloaded and ready to be installed.
 * @note If result is true, call installLocalUpdate,
 * @return True if an update is downloaded, false if partially downloaded.
 *
 * If an update was partially downloaded, the function will resume asynchronously and return false.
 */
bool AutoUpdater::isLocalUpdateReady()
{
    // Updates only for supported platforms
    if (platform.isEmpty())
        return false;

    if (isDownloadingUpdate)
        return false;

    // Check that there's an update dir in the first place, valid or not
    QString updateDirStr = Settings::getInstance().getSettingsDirPath() + "/update/";
    QDir updateDir(updateDirStr);
    if (!updateDir.exists())
        return false;

    // Check that we have a flist and generate a diff
    QFile updateFlistFile(updateDirStr+"flist");
    if (!updateFlistFile.open(QIODevice::ReadOnly))
        return false;
    QByteArray updateFlistData = updateFlistFile.readAll();
    updateFlistFile.close();

    QList<UpdateFileMeta> updateFlist = parseFlist(updateFlistData);
    QList<UpdateFileMeta> diff = genUpdateDiff(updateFlist);

    // Check that we have every file
    for (UpdateFileMeta fileMeta : diff)
    {
        if (!QFile::exists(updateDirStr+fileMeta.installpath))
            return false;

        QFile f(updateDirStr+fileMeta.installpath);
        if (f.size() != (int64_t)fileMeta.size)
            return false;
    }

    return true;
}

/**
 * @brief Launches the qTox updater to try to install the local update and exits immediately.
 *
 * @note Will not check that the update actually exists, use isLocalUpdateReady first for that.
 * The qTox updater will restart us after the update is done.
 * If we fail to start the qTox updater, we will delete the update and exit.
 */
void AutoUpdater::installLocalUpdate()
{
    qDebug() << "About to start the qTox updater to install a local update";

    // Prepare to delete the update if we fail so we don't fail again.
    auto failExit = []()
    {
        qCritical() << "Failed to start the qTox updater, removing the update and exiting";
        QString updateDirStr = Settings::getInstance().getSettingsDirPath() + "/update/";
        QDir(updateDirStr).removeRecursively();
        exit(-1);
    };

    // Updates only for supported platforms.
    if (platform.isEmpty())
        failExit();

    // Workaround QTBUG-7645
    // QProcess fails silently when elevation is required instead of showing a UAC prompt on Win7/Vista
#ifdef Q_OS_WIN
    QString modulePath = qApp->applicationDirPath().replace('/', '\\');
    HINSTANCE result = ::ShellExecuteW(0, L"open", updaterBin.toStdWString().c_str(),
                                       0, modulePath.toStdWString().c_str(), SW_SHOWNORMAL);
    if (result == (HINSTANCE)SE_ERR_ACCESSDENIED)
    {
        // Requesting elevation
        result = ::ShellExecuteW(0, L"runas", updaterBin.toStdWString().c_str(),
                                 0, modulePath.toStdWString().c_str(), SW_SHOWNORMAL);
    }
    if (result <= (HINSTANCE)32)
        failExit();
#else
    if (!QProcess::startDetached(updaterBin))
        failExit();
#endif

    exit(0);
}

/**
 * @brief Checks update an show dialog asking to download it.
 * @note Runs asynchronously in its own thread, and will return immediatly
 *
 * Will call isUpdateAvailable, and as such may processEvents.
 * Connects to the qTox update server, if an update is found
 * shows a dialog to the user asking to download it.
 */
void AutoUpdater::checkUpdatesAsyncInteractive()
{
    if (isDownloadingUpdate)
        return;

    QtConcurrent::run(&AutoUpdater::checkUpdatesAsyncInteractiveWorker);
}

/**
 * @brief Does the actual work for checkUpdatesAsyncInteractive
 *
 * Blocking, but otherwise has the same properties than checkUpdatesAsyncInteractive
 */
void AutoUpdater::checkUpdatesAsyncInteractiveWorker()
{
    if (!isUpdateAvailable())
        return;

    // If there's already an update dir, resume updating, otherwise ask the user
    QString updateDirStr = Settings::getInstance().getSettingsDirPath() + "/update/";
    QDir updateDir(updateDirStr);



    if (updateDir.exists() && QFile(updateDirStr+"flist").exists())
    {
        setProgressVersion(getUpdateVersion().versionString);
        downloadUpdate();
        return;
    }

    VersionInfo newVersion = getUpdateVersion();
    QString contentText = QObject::tr("An update is available, do you want to download it now?\n"
                                      "It will be installed when qTox restarts.");
    if (!newVersion.versionString.isEmpty())
        contentText += "\n\n" + QObject::tr("Version %1, %2").arg(newVersion.versionString,
                                QDateTime::fromMSecsSinceEpoch(newVersion.timestamp*1000).toString());


    if (abortFlag)
        return;

    if (GUI::askQuestion(QObject::tr("Update", "The title of a message box"),
                              contentText, true, false))
    {
        setProgressVersion(newVersion.versionString);
        GUI::showUpdateDownloadProgress();
        downloadUpdate();
    }
}

/**
 * @brief Thread safe setter
 * @param version Version to set.
 */
void AutoUpdater::setProgressVersion(QString version)
{
    QMutexLocker lock(&progressVersionMutex);
    progressVersion = version;
}

/**
 * @brief Abort update process.
 *
 * @note Aborting will make some functions try to return early.
 * Call before qTox exits to avoid the updater running in the background.
 */
void AutoUpdater::abortUpdates()
{
    abortFlag = true;
    isDownloadingUpdate = false;
}

/**
 * @brief Functions giving info on the progress of update downloads.
 * @return Version as string.
 */
QString AutoUpdater::getProgressVersion()
{
    QMutexLocker lock(&progressVersionMutex);
    return progressVersion;
}

int AutoUpdater::getProgressValue()
{
    return progressValue;
}
