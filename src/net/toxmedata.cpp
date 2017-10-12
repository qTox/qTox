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

#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include <ctime>

QByteArray ToxmeData::parsePublicKey(const QString& text) const
{
    const QJsonObject json = QJsonDocument::fromJson(text.toLatin1()).object();
    const QString& key = json[QStringLiteral("key")].toString();
    return QByteArray::fromHex(key.toLatin1());
}

QString ToxmeData::encryptedJson(int action, const QByteArray& pk, const QByteArray& encrypted,
                                 const QByteArray& nonce) const
{
    const QJsonObject json = {
        { QStringLiteral("action"), action },
        { QStringLiteral("public_key"), QString{pk.toHex()} },
        { QStringLiteral("encrypted"), QString{encrypted.toBase64()} },
        { QStringLiteral("nonce"), QString{nonce.toBase64()} },
    };

    return QJsonDocument{json}.toJson(QJsonDocument::Compact);
}

QString ToxmeData::lookupRequest(const QString& address) const
{
    const QJsonObject json = {
        { QStringLiteral("action"), 3 },
        { QStringLiteral("name"), address },
    };

    return QJsonDocument{json}.toJson(QJsonDocument::Compact);
}

ToxId ToxmeData::lookup(const QString& inText) const
{
    const QJsonObject json = QJsonDocument::fromJson(inText.toLatin1()).object();
    const QString& text = json[QStringLiteral("tox_id")].toString();
    return ToxId{text};
}

ToxmeData::ExecCode ToxmeData::extractCode(const QString& srcJson) const
{
    const QJsonObject json = QJsonDocument::fromJson(srcJson.toLatin1()).object();
    if (json.isEmpty()) {
        return ServerError;
    }

    const int code = json[QStringLiteral("c")].toInt(INT32_MAX);
    if (code == INT32_MAX) {
        return IncorrectResponse;
    }

    return ExecCode(code);
}

QString ToxmeData::createAddressRequest(const ToxId id, const QString& address, const QString& bio,
                                        bool keepPrivate) const
{
    const QJsonObject json = {
        { QStringLiteral("bio"), bio },
        { QStringLiteral("name"), address },
        { QStringLiteral("tox_id"), id.toString() },
        { QStringLiteral("privacy"), keepPrivate ? 0 : 2 },
        { QStringLiteral("timestamp"), static_cast<int>(time(nullptr)) },
    };

    return QJsonDocument{json}.toJson();
}

QString ToxmeData::getPass(const QString& srcJson, ToxmeData::ExecCode& code)
{
    const QJsonObject json = QJsonDocument::fromJson(srcJson.toLatin1()).object();
    if (json.isEmpty()) {
        code = ToxmeData::NoPassword;
        return QString{};
    }

    const QJsonValue pass = json[QStringLiteral("password")];
    if (pass.isNull()) {
        code = ToxmeData::Updated;
        return QString{};
    }

    if (!pass.isString()) {
        code = ToxmeData::IncorrectResponse;
        return QString{};
    }

    return pass.toString();
}

QString ToxmeData::deleteAddressRequest(const ToxPk& pk)
{
    QJsonObject json = {
        { QStringLiteral("public_key"), pk.toString() },
        { QStringLiteral("timestamp"), static_cast<int>(time(nullptr)) },
    };
    return QJsonDocument{json}.toJson();
}
