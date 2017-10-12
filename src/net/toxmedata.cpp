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

/**
 * @brief Get server public key from Json.
 * @param text Json text.
 * @return Server public key.
 */
QByteArray ToxmeData::parsePublicKey(const QString& text) const
{
    const QJsonObject json = QJsonDocument::fromJson(text.toLatin1()).object();
    const QString& key = json[QStringLiteral("key")].toString();
    return QByteArray::fromHex(key.toLatin1());
}

/**
 * @brief Build Json with encrypted payload.
 * @param action Action number.
 * @param pk User public key
 * @param encrypted Encrypted payload.
 * @param nonce Crypto nonce.
 * @return Json with action and encrypted payload.
 */
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

/**
 * @brief Build lookup request Json.
 * @param address Address to lookup.
 * @return Json to lookup.
 */
QString ToxmeData::lookupRequest(const QString& address) const
{
    const QJsonObject json = {
        { QStringLiteral("action"), 3 },
        { QStringLiteral("name"), address },
    };

    return QJsonDocument{json}.toJson(QJsonDocument::Compact);
}

/**
 * @brief Extract ToxId from lookup Json.
 * @param inText Json text.
 * @return User ToxId.
 */
ToxId ToxmeData::lookup(const QString& inText) const
{
    const QJsonObject json = QJsonDocument::fromJson(inText.toLatin1()).object();
    const QString& text = json[QStringLiteral("tox_id")].toString();
    return ToxId{text};
}

/**
 * @brief Extract toxme result code.
 * @param srcJson Json text.
 * @return Toxme code result.
 */
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

/**
 * @brief Build create address request Json.
 * @param id Self ToxId.
 * @param address Preferred address.
 * @param bio Self biography.
 * @param keepPrivate  If true, the address will not be published on toxme site.
 * @return Json to register Toxme address.
 */
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

/**
 * @brief Extrace password from Json answer.
 * @param srcJson[in] Json text.
 * @param code[out] Result code. Changed if password not extracted.
 * @return Extracted password.
 */
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

/**
 * @brief Build Json to delete address.
 * @param pk Self public key.
 * @return Json to delete address.
 */
QString ToxmeData::deleteAddressRequest(const ToxPk& pk)
{
    QJsonObject json = {
        { QStringLiteral("public_key"), pk.toString() },
        { QStringLiteral("timestamp"), static_cast<int>(time(nullptr)) },
    };
    return QJsonDocument{json}.toJson();
}
