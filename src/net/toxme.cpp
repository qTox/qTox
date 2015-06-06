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
#include <QtDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QCoreApplication>
#include <sodium/crypto_box.h>
#include <sodium/randombytes.h>
#include <string>
#include <ctime>

const QString Toxme::apiUrl{"https://toxme.se/api"};

const unsigned char Toxme::pinnedPk[] = {0x5D, 0x72, 0xC5, 0x17, 0xDF, 0x6A, 0xEC, 0x54, 0xF1, 0xE9, 0x77, 0xA6, 0xB6, 0xF2, 0x59, 0x14,
                0xEA, 0x4C, 0xF7, 0x27, 0x7A, 0x85, 0x02, 0x7C, 0xD9, 0xF5, 0x19, 0x6D, 0xF1, 0x7E, 0x0B, 0x13};

QByteArray Toxme::makeJsonRequest(QString json)
{
    QNetworkAccessManager netman;
    QNetworkRequest request{apiUrl};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = netman.post(request,json.toUtf8());
    while (!reply->isFinished())
        qApp->processEvents();

    return reply->readAll();
}

QByteArray Toxme::prepareEncryptedJson(int action, QString payload)
{
    QPair<QByteArray, QByteArray> keypair = Core::getInstance()->getKeypair();
    if (keypair.first.isEmpty() || keypair.second.isEmpty())
    {
        qWarning() << "prepareEncryptedJson: Couldn't get our keypair, aborting";
        return QByteArray();
    }

    QByteArray nonce(crypto_box_NONCEBYTES, 0);
    randombytes((uint8_t*)nonce.data(), crypto_box_NONCEBYTES);

    QByteArray payloadData = payload.toUtf8();
    const size_t cypherlen = crypto_box_MACBYTES+payloadData.size();
    unsigned char* payloadEnc = new unsigned char[cypherlen];

    crypto_box_easy(payloadEnc,(uint8_t*)payloadData.data(),payloadData.size(),
                    (uint8_t*)nonce.data(),pinnedPk,(uint8_t*)keypair.second.data());
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
    address.replace('\\',"\\\\");
    address.replace('"',"\"");

    ToxId id;
    const QString json{"{\"action\":3,\"name\":\""+address+"\"}"};
    static const QByteArray pattern{"public_key\""};

    QByteArray response = makeJsonRequest(json);
    const int index = response.indexOf(pattern);
    if (index == -1)
        return id;

    response = response.mid(index+pattern.size());

    const int idStart = response.indexOf('"');
    if (idStart == -1)
        return id;

    response = response.mid(idStart+1);

    const int idEnd = response.indexOf('"');
    if (idEnd == -1)
        return id;

    response.truncate(idEnd);

    id = ToxId(response);
    return id;
}

int Toxme::extractError(QString json)
{
    static const QByteArray pattern{"c\":"};

    json = json.remove(' ');
    const int index = json.indexOf(pattern);
    if (index == -1)
        return INT_MIN;

    json = json.mid(index+pattern.size());

    const int end = json.indexOf('}');
    if (end == -1)
        return INT_MIN;

    json.truncate(end);

    bool ok;
    int r = json.toInt(&ok);
    if (!ok)
        return INT_MIN;

    return r;
}

bool Toxme::createAddress(ToxId id, QString address,
                              bool keepPrivate, QString bio)
{
    int privacy = keepPrivate ? 0 : 2;
    // JSON injection ?
    bio.replace('\\',"\\\\");
    bio.replace('"',"\"");
    address.replace('\\',"\\\\");
    address.replace('"',"\"");

    const QString payload{"{\"tox_id\":\""+id.toString()+"\","
                          "\"name\":\""+address+"\","
                          "\"privacy\":"+QString().setNum(privacy)+","
                          "\"bio\":\""+bio+"\","
                          "\"timestamp\":"+QString().setNum(time(0))+"}"};

    QByteArray response = makeJsonRequest(prepareEncryptedJson(1,payload));

    return (extractError(response) == 0);
}

bool Toxme::deleteAddress(ToxId id)
{
    const QString payload{"{\"public_key\":\""+id.toString().left(64)+"\","
                          "\"timestamp\":"+QString().setNum(time(0))+"}"};

    QByteArray response = makeJsonRequest(prepareEncryptedJson(2,payload));

    return (extractError(response) == 0);
}
