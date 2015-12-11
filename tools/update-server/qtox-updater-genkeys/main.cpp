#include <sodium.h>
#include <QCoreApplication>
#include <QByteArray>
#include <QFile>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    (void) a;

    QByteArray skey(crypto_sign_SECRETKEYBYTES, 0);
    QFile skeyFile("qtox-updater-skey");
    if (!skeyFile.open(QIODevice::WriteOnly))
    {
        qCritical() << "Failed to open qtox-updater-skey";
        return 1;
    }

    QByteArray pkey(crypto_sign_PUBLICKEYBYTES, 0);
    QFile pkeyFile("qtox-updater-pkey");
    if (!pkeyFile.open(QIODevice::WriteOnly))
    {
        qCritical() << "Failed to open qtox-updater-pkey";
        return 1;
    }

    crypto_sign_keypair((uint8_t*)pkey.data(), (uint8_t*)skey.data());
    skeyFile.write(skey);
    pkeyFile.write(pkey);

    qDebug() << "Wrote new keys to disk";
    return 0;
}

