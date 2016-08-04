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

class Toxme
{
public:
    enum ExecCode {
        ExecError = -50,
        Ok = 0,
        Updated = 1,
        ServerError = 2,
        IncorrectResponse = 3,
        NoPassword = 4
    };

    static ToxId lookup(QString address);
    static QString createAddress(ExecCode &code, QString server, ToxId id, QString address,
                              bool keepPrivate=true, QString bio=QString());
    static ExecCode deleteAddress(QString server, ToxId id);
    static QString getErrorMessage(int errorCode);
    static QString translateErrorMessage(int errorCode);

private:
    Toxme() = delete;
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
