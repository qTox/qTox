/*
    Copyright © 2015 by The qTox Project

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


#ifndef TOXME_H
#define TOXME_H

#include <QString>
#include <QMap>
#include <QMutex>
#include <QNetworkReply>
#include <memory>
#include "src/core/toxid.h"

class QNetworkAccessManager;

/// This class implements a client for the toxme.se API
/// The class is thread safe
/// May process events while waiting for blocking calls
class Toxme
{
public:
    enum ExecCode {
        ExecError = -50,
        Registered = 0,
        Updated = 1,
        ServerError = 2,
        IncorrectResponce = 3,
        NoPassword = 4
    };

    /// Converts a toxme.se address to a Tox ID, returns an empty ID on error
    static ToxId lookup(QString address);
    /// Creates a new toxme.se address associated with a Tox ID.
    /// If keepPrivate, the address will not be published on toxme.se
    /// The bio is a short optional description of yourself if you want to publish your address.
    /// If it passed without error, return password, else return errorCode in QString
    static QString createAddress(ExecCode &code, QString server, ToxId id, QString address,
                              bool keepPrivate=true, QString bio=QString());
    /// Deletes the address associated with your current Tox ID
    static int deleteAddress(QString server, ToxId id);
    /// Return string of the corresponding error code
    static QString getErrorMessage(int errorCode);

private:
    Toxme()=delete;
    static QByteArray makeJsonRequest(QString url, QString json, QNetworkReply::NetworkError &error);
    static QByteArray prepareEncryptedJson(QString url, int action, QString payload);
    static QByteArray getServerPubkey(QString url, QNetworkReply::NetworkError &error);
    static QString getPass(QString json, ExecCode &code);
    static ExecCode extractError(QString json);

private:
    static const QMap<QString, QString> pubkeyUrls;
    static const QMap<QString, QString> apiUrls;
};

#endif // TOXME_H
