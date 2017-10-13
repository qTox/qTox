/*
    Copyright © 2017 by The qTox Project Contributors

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

#include "toxmedata.h"
#include "src/core/toxid.h"

#include <QString>

#include <ctime>

QByteArray ToxmeData::parsePublicKey(const QString& text) const
{
    static const QByteArray pattern{"key\":\""};

    QString json = text;
    json.remove(' ');
    int start = json.indexOf(pattern) + pattern.length();
    int end = json.indexOf("\"", start);
    int pubkeySize = (end - start) / 2;
    QString rawKey = json.mid(start, pubkeySize * 2);

    QByteArray key;
    // I think, exist more easy way to convert key to ByteArray
    for (int i = 0; i < pubkeySize; ++i) {
        QString byte = rawKey.mid(i * 2, 2);
        key[i] = byte.toInt(nullptr, 16);
    }

    return key;
}

QString ToxmeData::encryptedJson(int action, const QByteArray& pk, const QByteArray& encrypted,
                                 const QByteArray& nonce) const
{
    return "{\"action\":" + QString().setNum(action) +
            ",\"public_key\":\"" + pk.toHex() +
            "\",\"encrypted\":\"" + encrypted.toBase64() +
            "\",\"nonce\":\"" + nonce.toBase64() + "\"}";
}

QString ToxmeData::lookupRequest(const QString& address) const
{
    return "{\"action\":3,\"name\":\"" + address + "\"}";
}

ToxId ToxmeData::lookup(const QString& inText) const
{
    QString text = inText;
    static const QByteArray pattern{"tox_id\""};
    const int index = text.indexOf(pattern);
    if (index == -1)
        return ToxId();

    text = text.mid(index + pattern.size());

    const int idStart = text.indexOf('"');
    if (idStart == -1)
        return ToxId();

    text = text.mid(idStart + 1);

    const int idEnd = text.indexOf('"');
    if (idEnd == -1)
        return ToxId();

    text.truncate(idEnd);

    return ToxId(text);
}

ToxmeData::ExecCode ToxmeData::extractCode(const QString& srcJson) const
{
    QString json = srcJson;
    static const QByteArray pattern{"c\":"};

    if (json.isEmpty())
        return ServerError;

    json = json.remove(' ');
    const int start = json.indexOf(pattern);
    if (start == -1)
        return ServerError;

    json = json.mid(start + pattern.size());
    int end = json.indexOf(",");
    if (end == -1) {
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

QString ToxmeData::createAddressRequest(const ToxId id, const QString& address, const QString& bio,
                                        bool keepPrivate) const
{
    int privacy = keepPrivate ? 0 : 2;
    return "{\"tox_id\":\"" + id.toString() +
           "\",\"name\":\"" + address +
           "\",\"privacy\":" + QString().setNum(privacy) +
           ",\"bio\":\"" + bio +
           "\",\"timestamp\":" + QString().setNum(time(nullptr)) +
            "}";

}

QString ToxmeData::getPass(const QString& srcJson, ToxmeData::ExecCode& code)
{
    QString json = srcJson;
    static const QByteArray pattern{"password\":"};

    json = json.remove(' ');
    const int start = json.indexOf(pattern);
    if (start == -1) {
        code = ToxmeData::NoPassword;
        return QString();
    }

    json = json.mid(start + pattern.size());
    if (json.startsWith("null")) {
        code = ToxmeData::Updated;
        return QString();
    }

    json = json.mid(1, json.length());
    int end = json.indexOf("\"");
    if (end == -1) {
        code = ToxmeData::IncorrectResponse;
        return QString();
    }

    json.truncate(end);

    return json;
}

QString ToxmeData::deleteAddressRequest(const ToxPk& pk)
{
    return "{\"public_key\":\"" + pk.toString() +
           "\",\"timestamp\":" + QString().setNum(time(nullptr)) +
           "}";

}
