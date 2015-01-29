#ifndef TOXME_H
#define TOXME_H

#include <QString>
#include <QMutex>
#include "corestructs.h"

class QNetworkAccessManager;

/// This class implements a client for the toxme.se API
/// The class is thread safe
/// May process events while waiting for blocking calls
class Toxme
{
public:
    /// Converts a toxme.se address to a Tox ID, returns an empty ID on error
    static ToxID lookup(QString address);
    /// Creates a new toxme.se address associated with a Tox ID.
    /// If keepPrivate, the address will not be published on toxme.se
    /// The bio is a short optional description of yourself if you want to publish your address.
    static bool createAddress(ToxID id, QString address,
                              bool keepPrivate=true, QString bio=QString());

private:
    Toxme()=delete;
    static QByteArray makeJsonRequest(QString json);
    static QByteArray prepareEncryptedJson(int action, QString payload);
    static void incrementNonce(unsigned char nonce[]);

private:
    static const QString apiUrl;
};

#endif // TOXME_H
