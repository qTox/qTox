#include "src/autoupdate.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QCoreApplication>

#include <sodium.h>

#ifdef _WIN32
const QString AutoUpdater::platform = "win32";
#else
const QString AutoUpdater::platform = "win32"; ///TODO: FIXME: undefine, we want an empty qstring
#endif
const QString AutoUpdater::updateServer = "http://127.0.0.1";
const QString AutoUpdater::checkURI = AutoUpdater::updateServer+"/qtox/"+AutoUpdater::platform+"/version";
const QString AutoUpdater::flistURI = AutoUpdater::updateServer+"/qtox/"+AutoUpdater::platform+"/flist";
const QString AutoUpdater::filesURI = AutoUpdater::updateServer+"/qtox/"+AutoUpdater::platform+"/files/";
unsigned char AutoUpdater::key[crypto_sign_PUBLICKEYBYTES] =
{
    0xa5, 0x80, 0xf3, 0xb7, 0xd0, 0x10, 0xc0, 0xf9, 0xd6, 0xcf, 0x48, 0x15, 0x99, 0x70, 0x92, 0x49,
    0xf6, 0xe8, 0xe5, 0xe2, 0x6c, 0x73, 0x8c, 0x48, 0x25, 0xed, 0x01, 0x72, 0xf7, 0x6c, 0x17, 0x28
};

bool AutoUpdater::isUpdateAvailable()
{
    QString newVersion = getUpdateVersion();
    if (newVersion.isEmpty() || newVersion == GIT_VERSION)
        return false;
    else
        return true;
}

QString AutoUpdater::getUpdateVersion()
{
    QString version;

    // Updates only for supported platforms
    if (platform.isEmpty())
        return version;

    QNetworkAccessManager *manager = new QNetworkAccessManager;

    QNetworkReply* reply = manager->get(QNetworkRequest(QUrl(checkURI)));
    while (!reply->isFinished())
        qApp->processEvents();

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "AutoUpdater: getUpdateVersion: network error: "<<reply->errorString();
        return version;
    }

    QByteArray data = reply->readAll();

    // Check updater protocol version
    if ((int)data[0] != '1')
    {
        qWarning() << "AutoUpdater: getUpdateVersion: Bad protocol version "<<(uint8_t)data[0];
        return version;
    }

    // Check the signature
    QByteArray sigData = data.mid(1, crypto_sign_BYTES);
    unsigned char* sig = (unsigned char*)sigData.data();
    QByteArray msgData = data.mid(1+crypto_sign_BYTES);
    unsigned char* msg = (unsigned char*)msgData.data();

    if (crypto_sign_verify_detached(sig, msg, msgData.size(), key) != 0)
    {
        qCritical() << "AutoUpdater: getUpdateVersion: RECEIVED FORGED VERSION FILE FROM "<<updateServer;
        return version;
    }

    version = msgData;

    return version;
}
