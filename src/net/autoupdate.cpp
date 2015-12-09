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


#include "src/net/autoupdate.h"
#include "src/persistence/serialize.h"
#include "src/persistence/settings.h"
#include "src/widget/widget.h"
#include "src/widget/gui.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QtConcurrent/QtConcurrent>
#include <QMessageBox>

#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>
#endif

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
    0xd6, 0x32, 0x8a, 0x08, 0xf9, 0xed, 0x14, 0x8a, 0xad, 0x97, 0xa1, 0x33, 0xc5, 0xe2, 0x6c, 0x77,
    0x3b, 0x51, 0xa2, 0x70, 0xc2, 0xd5, 0xc1, 0x5e, 0x31, 0xda, 0x2a, 0x39, 0x9e, 0xf6, 0x99, 0x7e
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
const QString AutoUpdater::checkURI = AutoUpdater::updateServer+"/qtox/"+AutoUpdater::platform+"/version";
const QString AutoUpdater::flistURI = AutoUpdater::updateServer+"/qtox/"+AutoUpdater::platform+"/flist";
const QString AutoUpdater::filesURI = AutoUpdater::updateServer+"/qtox/"+AutoUpdater::platform+"/files/";
bool AutoUpdater::abortFlag{false};
std::atomic_bool AutoUpdater::isDownloadingUpdate{false};

bool AutoUpdater::isUpdateAvailable()
{
    if (isDownloadingUpdate)
        return false;

    VersionInfo newVersion = getUpdateVersion();
    if (newVersion.timestamp <= TIMESTAMP
            || newVersion.versionString.isEmpty() || newVersion.versionString == GIT_VERSION)
        return false;
    else
        return true;
}

AutoUpdater::VersionInfo AutoUpdater::getUpdateVersion()
{
    VersionInfo versionInfo;
    versionInfo.timestamp = 0;

    // Updates only for supported platforms
    if (platform.isEmpty())
        return versionInfo;

    QNetworkAccessManager *manager = new QNetworkAccessManager;
    QNetworkReply* reply = manager->get(QNetworkRequest(QUrl(checkURI)));
    while (!reply->isFinished())
        qApp->processEvents();

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

QByteArray AutoUpdater::getUpdateFlist()
{
    QByteArray flist;

    QNetworkAccessManager *manager = new QNetworkAccessManager;
    QNetworkReply* reply = manager->get(QNetworkRequest(QUrl(flistURI)));
    while (!reply->isFinished())
        qApp->processEvents();

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

QByteArray AutoUpdater::getLocalFlist()
{
    QByteArray flist;

    QFile flistFile("flist");
    if (!flistFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "getLocalFlist: Can't open local flist";
        return flist;
    }

    flist = flistFile.readAll();
    flistFile.close();

    return flist;
}

QList<AutoUpdater::UpdateFileMeta> AutoUpdater::genUpdateDiff(QList<UpdateFileMeta> updateFlist)
{
    QList<UpdateFileMeta> diff;
    QList<UpdateFileMeta> localFlist = parseFlist(getLocalFlist());

    for (UpdateFileMeta file : updateFlist)
        if (!localFlist.contains(file))
            diff += file;

    return diff;
}

AutoUpdater::UpdateFile AutoUpdater::getUpdateFile(UpdateFileMeta fileMeta)
{
    UpdateFile file;
    file.metadata = fileMeta;

    QNetworkAccessManager *manager = new QNetworkAccessManager;
    QNetworkReply* reply = manager->get(QNetworkRequest(QUrl(filesURI+fileMeta.id)));
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

    if (abortFlag)
    {
        isDownloadingUpdate = false;
        return false;
    }

    qDebug() << "Need to update " << diff.size() << " files";

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

    // Download and write each new file
    for (UpdateFileMeta fileMeta : diff)
    {
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
            continue;
        }

        qDebug() << "Downloading '" + fileMeta.installpath + "' ...";

        // Create subdirs if necessary
        QString fileDirStr{QFileInfo(updateDirStr+fileMeta.installpath).absolutePath()};
        if (!QDir(fileDirStr).exists())
            QDir().mkpath(fileDirStr);

        // Download
        UpdateFile file = getUpdateFile(fileMeta);
        if (abortFlag)
        {
            isDownloadingUpdate = false;
            return false;
        }
        if (file.data.isNull())
        {
            qCritical() << "downloadUpdate: Error downloading a file, aborting...";
            isDownloadingUpdate = false;
            return false;
        }

        // Check signature
        if (crypto_sign_verify_detached(file.metadata.sig, (unsigned char*)file.data.data(),
                                        file.data.size(), key) != 0)
        {
            qCritical() << "downloadUpdate: RECEIVED FORGED FILE, aborting...";
            isDownloadingUpdate = false;
            return false;
        }

        // Save
        if (!fileFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            qCritical() << "downloadUpdate: Can't save new update file, aborting...";
            isDownloadingUpdate = false;
            return false;
        }
        fileFile.write(file.data);
        fileFile.close();
    }

    qDebug() << "downloadUpdate: The update is ready, it'll be installed on the next restart";

    isDownloadingUpdate = false;
    return true;
}

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

void AutoUpdater::installLocalUpdate()
{
    qDebug() << "About to start the qTox updater to install a local update";

    // Delete the update if we fail so we don't fail again.

    // Updates only for supported platforms.
    if (platform.isEmpty())
    {
        qCritical() << "Failed to start the qTox updater, removing the update and exiting";
        QString updateDirStr = Settings::getInstance().getSettingsDirPath() + "/update/";
        QDir(updateDirStr).removeRecursively();
        exit(-1);
    }

    // Workaround QTBUG-7645
    // QProcess fails silently when elevation is required instead of showing a UAC prompt on Win7/Vista
#ifdef Q_OS_WIN
    HINSTANCE result = ::ShellExecuteA(0, "open", updaterBin.toUtf8().constData(), 0, 0, SW_SHOWNORMAL);
    if (result == (HINSTANCE)SE_ERR_ACCESSDENIED)
    {
        // Requesting elevation
        result = ::ShellExecuteA(0, "runas", updaterBin.toUtf8().constData(), 0, 0, SW_SHOWNORMAL);
    }
    if (result <= (HINSTANCE)32)
    {
        goto fail;
    }
#else
    if (!QProcess::startDetached(updaterBin))
        goto fail;
#endif

    exit(0);

    // Centralized error handling
fail:
    qCritical() << "Failed to start the qTox updater, removing the update and exiting";
    QString updateDirStr = Settings::getInstance().getSettingsDirPath() + "/update/";
    QDir(updateDirStr).removeRecursively();
    exit(-1);
}

void AutoUpdater::checkUpdatesAsyncInteractive()
{
    if (isDownloadingUpdate)
        return;

    QtConcurrent::run(&AutoUpdater::checkUpdatesAsyncInteractiveWorker);
}

void AutoUpdater::checkUpdatesAsyncInteractiveWorker()
{
    if (!isUpdateAvailable())
        return;

    // If there's already an update dir, resume updating, otherwise ask the user
    QString updateDirStr = Settings::getInstance().getSettingsDirPath() + "/update/";
    QDir updateDir(updateDirStr);

    if ((updateDir.exists() && QFile(updateDirStr+"flist").exists())
            || GUI::askQuestion(QObject::tr("Update", "The title of a message box"),
        QObject::tr("An update is available, do you want to download it now?\nIt will be installed when qTox restarts."), true, false))
    {
        downloadUpdate();
    }
}

void AutoUpdater::abortUpdates()
{
    abortFlag = true;
    isDownloadingUpdate = false;
}
