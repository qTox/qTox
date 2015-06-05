#ifndef TOXME_H
#define TOXME_H

#include <QString>
#include <QMutex>
#include <memory>
#include "src/core/toxid.h"

class QNetworkAccessManager;

/// This class implements a client for the toxme.se API
/// The class is thread safe
/// May process events while waiting for blocking calls
class Toxme
{
public:
    /// Converts a toxme.se address to a Tox ID, returns an empty ID on error
    static ToxId lookup(QString address);
    /// Creates a new toxme.se address associated with a Tox ID.
    /// If keepPrivate, the address will not be published on toxme.se
    /// The bio is a short optional description of yourself if you want to publish your address.
    static bool createAddress(ToxId id, QString address,
                              bool keepPrivate=true, QString bio=QString());
    /// Deletes the address associated with your current Tox ID
    static bool deleteAddress(ToxId id);

private:
    Toxme()=delete;
    static QByteArray makeJsonRequest(QString json);
    static QByteArray prepareEncryptedJson(int action, QString payload);
    static int extractError(QString json);

private:
    static const QString apiUrl;
    static const unsigned char pinnedPk[];
};

#endif // TOXME_H
