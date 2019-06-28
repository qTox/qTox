/*
    Copyright © 2017-2019 by The qTox Project Contributors

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

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

namespace {
namespace consts {
namespace keys {
const QString Key{QStringLiteral("key")};
const QString Action{QStringLiteral("action")};
const QString PublicKey{QStringLiteral("public_key")};
const QString Encrypted{QStringLiteral("encrypted")};
const QString Nonce{QStringLiteral("nonce")};
const QString Name{QStringLiteral("name")};
const QString ToxId{QStringLiteral("tox_id")};
const QString Code{QStringLiteral("c")};
const QString Bio{QStringLiteral("bio")};
const QString Privacy{QStringLiteral("privacy")};
const QString Timestamp{QStringLiteral("timestamp")};
const QString Password{QStringLiteral("password")};
}
}
}

static qint64 getCurrentTime()
{
    return QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() / 1000;
}

/**
 * @brief Get server public key from Json.
 * @param text Json text.
 * @return Server public key.
 */
QByteArray ToxmeData::parsePublicKey(const QString& text) const
{
    const QJsonObject json = QJsonDocument::fromJson(text.toLatin1()).object();
    const QString& key = json[consts::keys::Key].toString();
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
        { consts::keys::Action, action },
        { consts::keys::PublicKey, QString{pk.toHex()} },
        { consts::keys::Encrypted, QString{encrypted.toBase64()} },
        { consts::keys::Nonce, QString{nonce.toBase64()} },
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
        { consts::keys::Action, 3 },
        { consts::keys::Name, address },
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
    const QString& text = json[consts::keys::ToxId].toString();
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

    const int code = json[consts::keys::Code].toInt(INT32_MAX);
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
        { consts::keys::Bio, bio },
        { consts::keys::Name, address },
        { consts::keys::ToxId, id.toString() },
        { consts::keys::Privacy, keepPrivate ? 0 : 2 },
        { consts::keys::Timestamp, getCurrentTime() },
    };

    return QJsonDocument{json}.toJson(QJsonDocument::Compact);
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

    const QJsonValue pass = json[consts::keys::Password];
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
        { consts::keys::PublicKey, pk.toString() },
        { consts::keys::Timestamp, getCurrentTime() },
    };
    return QJsonDocument{json}.toJson(QJsonDocument::Compact);
}
