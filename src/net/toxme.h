/*
    Copyright © 2015-2019 by The qTox Project Contributors

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

#include "src/core/toxid.h"
#include "src/net/toxmedata.h"
#include <QMap>
#include <QMutex>
#include <QNetworkReply>
#include <QString>
#include <memory>

class QNetworkAccessManager;

class Toxme
{
public:
    static ToxId lookup(QString address);
    static QString createAddress(ToxmeData::ExecCode& code, QString server, ToxId id,
                                 QString address, bool keepPrivate = true, QString bio = QString());
    static ToxmeData::ExecCode deleteAddress(QString server, ToxPk id);
    static QString getErrorMessage(int errorCode);
    static QString translateErrorMessage(int errorCode);

private:
    Toxme() = delete;
    static QByteArray makeJsonRequest(QString url, QString json, QNetworkReply::NetworkError& error);
    static QByteArray prepareEncryptedJson(QString url, int action, QString payload);
    static QByteArray getServerPubkey(QString url, QNetworkReply::NetworkError& error);
    static ToxmeData::ExecCode extractError(QString json);

private:
    static const QMap<QString, QString> pubkeyUrls;
    static const QMap<QString, QString> apiUrls;
};

#endif // TOXME_H
