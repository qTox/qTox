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

#include "toxme.h"
#include "src/core/core.h"
#include <src/persistence/settings.h>
#include <QtDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QCoreApplication>
#include <QThread>
#include <sodium/crypto_box.h>
#include <sodium/randombytes.h>
#include <string>
#include <ctime>

QByteArray Toxme::makeJsonRequest(QString url, QString json, QNetworkReply::NetworkError &error)
{
    if (error)
        return QByteArray();

    QNetworkAccessManager netman;
    netman.setProxy(Settings::getInstance().getProxy());
    QNetworkRequest request{url};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* reply = netman.post(request, json.toUtf8());

    while (!reply->isFinished())
    {
        QThread::msleep(1);
        qApp->processEvents();
    }

    error = reply->error();
    if (error)
    {
        qWarning() << "makeJsonRequest: A network error occured:" << reply->errorString();
        return QByteArray();
    }

    return reply->readAll();
}

QByteArray Toxme::getServerPubkey(QString url, QNetworkReply::NetworkError &error)
{
    if (error)
        return QByteArray();

    // Get key
    QNetworkAccessManager netman;
    netman.setProxy(Settings::getInstance().getProxy());
    QNetworkRequest request{url};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* reply = netman.get(request);

    while (!reply->isFinished())
    {
        QThread::msleep(1);
        qApp->processEvents();
    }

    error = reply->error();
    if (error)
    {
        qWarning() << "getServerPubkey: A network error occured:" << reply->errorString();
        return QByteArray();
    }

    // Extract key
    static const QByteArray pattern{"key\":\""};

    QString json = reply->readAll();
    delete reply;
    json = json.remove(' ');
    int start = json.indexOf(pattern) + pattern.length();
    int end = json.indexOf("\"", start);
    int pubkeySize = (end - start) / 2;
    QString rawKey = json.mid(start, pubkeySize*2);

    QByteArray key;
    // I think, exist more easy way to convert key to ByteArray
    for (int i = 0; i < pubkeySize; i++) {
        QString byte = rawKey.mid(i*2, 2);
        key[i] = byte.toInt(nullptr, 16);
    }

    return key;
}

QByteArray Toxme::prepareEncryptedJson(QString url, int action, QString payload)
{
    QPair<QByteArray, QByteArray> keypair = Core::getInstance()->getKeypair();
    if (keypair.first.isEmpty() || keypair.second.isEmpty())
    {
        qWarning() << "prepareEncryptedJson: Couldn't get our keypair, aborting";
        return QByteArray();
    }

    QNetworkReply::NetworkError error = QNetworkReply::NoError;
    QByteArray key = getServerPubkey(url, error);
    if (error != QNetworkReply::NoError)
        return QByteArray();

    QByteArray nonce(crypto_box_NONCEBYTES, 0);
    randombytes((uint8_t*)nonce.data(), crypto_box_NONCEBYTES);

    QByteArray payloadData = payload.toUtf8();
    const size_t cypherlen = crypto_box_MACBYTES+payloadData.size();
    unsigned char* payloadEnc = new unsigned char[cypherlen];

    int cryptResult = crypto_box_easy(payloadEnc,(uint8_t*)payloadData.data(),payloadData.size(),
                    (uint8_t*)nonce.data(),(unsigned char*)key.constData(),
                    (uint8_t*)keypair.second.data());

    if (cryptResult != 0) // error
        return QByteArray();

    QByteArray payloadEncData(reinterpret_cast<char*>(payloadEnc), cypherlen);
    delete[] payloadEnc;

    const QString json{"{\"action\":"+QString().setNum(action)+","
                       "\"public_key\":\""+keypair.first.toHex()+"\","
                       "\"encrypted\":\""+payloadEncData.toBase64()+"\","
                       "\"nonce\":\""+nonce.toBase64()+"\"}"};
    return json.toUtf8();
}

ToxId Toxme::lookup(QString address)
{
    // JSON injection ?
    address = address.trimmed();
    address.replace('\\',"\\\\");
    address.replace('"',"\"");

    const QString json{"{\"action\":3,\"name\":\""+address+"\"}"};

    QString apiUrl = "https://" + address.split(QLatin1Char('@')).last() + "/api";
    QNetworkReply::NetworkError error = QNetworkReply::NoError;
    QByteArray response = makeJsonRequest(apiUrl, json, error);

    if (error != QNetworkReply::NoError)
        return ToxId();

    static const QByteArray pattern{"tox_id\""};
    const int index = response.indexOf(pattern);
    if (index == -1)
        return ToxId();

    response = response.mid(index+pattern.size());

    const int idStart = response.indexOf('"');
    if (idStart == -1)
        return ToxId();

    response = response.mid(idStart+1);

    const int idEnd = response.indexOf('"');
    if (idEnd == -1)
        return ToxId();

    response.truncate(idEnd);

    return ToxId(response);
}

Toxme::ExecCode Toxme::extractError(QString json)
{
    static const QByteArray pattern{"c\":"};

    if (json.isEmpty())
        return ServerError;

    json = json.remove(' ');
    const int start = json.indexOf(pattern);
    if (start == -1)
        return ServerError;

    json = json.mid(start+pattern.size());
    int end = json.indexOf(",");
    if (end == -1)
    {
        end = json.indexOf("}");
        if (end == -1)
            return IncorrectResponse;
    }

    json.truncate(end);
    bool ok;
    int r = json.toInt(&ok);
    if (!ok)
        return IncorrectResponse;

    return ExecCode(r);
}

QString Toxme::createAddress(ExecCode &code, QString server, ToxId id, QString address,
                             bool keepPrivate, QString bio)
{
    int privacy = keepPrivate ? 0 : 2;
    // JSON injection ?
    bio.replace('\\',"\\\\");
    bio.replace('"',"\"");

    address.replace('\\',"\\\\");
    address.replace('"',"\"");

    bio = bio.trimmed();
    address = address.trimmed();
    server = server.trimmed();
    if (!server.contains("://"))
        server = "https://" + server;

    const QString payload{"{\"tox_id\":\""+id.toString()+"\","
                          "\"name\":\""+address+"\","
                          "\"privacy\":"+QString().setNum(privacy)+","
                          "\"bio\":\""+bio+"\","
                          "\"timestamp\":"+QString().setNum(time(0))+"}"};

    qDebug() << payload;
    QString pubkeyUrl = server + "/pk";
    QString apiUrl =  server + "/api";
    QNetworkReply::NetworkError error = QNetworkReply::NoError;
    QByteArray response = makeJsonRequest(apiUrl, prepareEncryptedJson(pubkeyUrl, 1, payload), error);
    qDebug() << response;

    code = extractError(response);
    if ((code != Ok && code != Updated) || error != QNetworkReply::NoError)
        return QString();

    return getPass(response, code);
}

QString Toxme::getPass(QString json, ExecCode &code) {
    static const QByteArray pattern{"password\":"};

    json = json.remove(' ');
    const int start = json.indexOf(pattern);
    if (start == -1)
    {
        code = NoPassword;
        return QString();
    }

    json = json.mid(start+pattern.size());
    if (json.startsWith("null"))
    {
        code = Updated;
        return QString();
    }

    json = json.mid(1, json.length());
    int end = json.indexOf("\"");
    if (end == -1)
    {
        code = IncorrectResponse;
        return QString();
    }

    json.truncate(end);

    return json;
}

Toxme::ExecCode Toxme::deleteAddress(QString server, ToxId id)
{
    const QString payload{"{\"public_key\":\""+id.toString().left(64)+"\","
                          "\"timestamp\":"+QString().setNum(time(0))+"}"};

    server = server.trimmed();
    if (!server.contains("://"))
        server = "https://" + server;

    qDebug() << payload;
    QString pubkeyUrl = server + "/pk";
    QString apiUrl = server + "/api";
    QNetworkReply::NetworkError error = QNetworkReply::NoError;
    QByteArray response = makeJsonRequest(apiUrl, prepareEncryptedJson(pubkeyUrl, 2, payload), error);

    return extractError(response);
}

QString Toxme::getErrorMessage(int errorCode)
{
    switch (errorCode) {
    case IncorrectResponse:
        return QObject::tr("Incorrect response");
    case NoPassword:
        return QObject::tr("No password in response");
    case ServerError:
        return QObject::tr("Server doesn't support Toxme");
    case -1:
        return QObject::tr("You must send POST requests to /api");
    case -2:
        return QObject::tr("Please try again using a HTTPS connection");
    case -3:
        return QObject::tr("I was unable to read your encrypted payload");
    case -4:
        return QObject::tr("You're making too many requests. Wait an hour and try again");
    case -25:
        return QObject::tr("This name is already in use");
    case -26:
        return QObject::tr("This Tox ID is already registered under another name");
    case -27:
        return QObject::tr("Please don't use a space in your name");
    case -28:
        return QObject::tr("Password incorrect");
    case -29:
        return QObject::tr("You can't use this name");
    case -30:
        return QObject::tr("Name not found");
    case -31:
        return QObject::tr("Tox ID not sent");
    case -41:
        return QObject::tr("Lookup failed because the server replied with invalid data");
    case -42:
        return QObject::tr("That user does not exist");
    case -43:
        return QObject::tr("Internal lookup error. Please file a bug");
    default:
        return QObject::tr("Unknown error (%1)").arg(errorCode);
    }
}
