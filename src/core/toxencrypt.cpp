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

#include "toxencrypt.h"
#include <tox/toxencryptsave.h>

#include <QByteArray>
#include <QDebug>
#include <QString>
#include <memory>

namespace {
/**
 * @brief Gets the error string for Tox_Err_Key_Derivation errors.
 * @param error The error number.
 * @return The verbose error message.
 */
QString getKeyDerivationError(Tox_Err_Key_Derivation error)
{
    switch (error) {
    case TOX_ERR_KEY_DERIVATION_OK:
        return QStringLiteral("The function returned successfully.");
    case TOX_ERR_KEY_DERIVATION_NULL:
        return QStringLiteral(
            "One of the arguments to the function was NULL when it was not expected.");
    case TOX_ERR_KEY_DERIVATION_FAILED:
        return QStringLiteral(
            "The crypto lib was unable to derive a key from the given passphrase.");
    default:
        return QStringLiteral("Unknown key derivation error.");
    }
}

/**
 * @brief Gets the error string for Tox_Err_Encryption errors.
 * @param error The error number.
 * @return The verbose error message.
 */
QString getEncryptionError(Tox_Err_Encryption error)
{
    switch (error) {
    case TOX_ERR_ENCRYPTION_OK:
        return QStringLiteral("The function returned successfully.");
    case TOX_ERR_ENCRYPTION_NULL:
        return QStringLiteral(
            "One of the arguments to the function was NULL when it was not expected.");
    case TOX_ERR_ENCRYPTION_KEY_DERIVATION_FAILED:
        return QStringLiteral(
            "The crypto lib was unable to derive a key from the given passphrase.");
    case TOX_ERR_ENCRYPTION_FAILED:
        return QStringLiteral("The encryption itself failed.");
    default:
        return QStringLiteral("Unknown encryption error.");
    }
}

/**
 * @brief Gets the error string for Tox_Err_Decryption errors.
 * @param error The error number.
 * @return The verbose error message.
 */
QString getDecryptionError(Tox_Err_Decryption error)
{
    switch (error) {
    case TOX_ERR_DECRYPTION_OK:
        return QStringLiteral("The function returned successfully.");
    case TOX_ERR_DECRYPTION_NULL:
        return QStringLiteral(
            "One of the arguments to the function was NULL when it was not expected.");
    case TOX_ERR_DECRYPTION_INVALID_LENGTH:
        return QStringLiteral(
            "The input data was shorter than TOX_PASS_ENCRYPTION_EXTRA_LENGTH bytes.");
    case TOX_ERR_DECRYPTION_BAD_FORMAT:
        return QStringLiteral("The input data is missing the magic number or is corrupted.");
    case TOX_ERR_DECRYPTION_KEY_DERIVATION_FAILED:
        return QStringLiteral("The crypto lib was unable to derive a key from the given passphrase.");
    case TOX_ERR_DECRYPTION_FAILED:
        return QStringLiteral("Decryption failed. Either the data was corrupted or the password/key was incorrect.");
    default:
        return QStringLiteral("Unknown decryption error.");
    }
}

/**
 * @brief Gets the error string for Tox_Err_Get_Salt errors.
 * @param error The error number.
 * @return The verbose error message.
 */
QString getSaltError(Tox_Err_Get_Salt error)
{
    switch (error) {
    case TOX_ERR_GET_SALT_OK:
        return QStringLiteral("The function returned successfully.");
    case TOX_ERR_GET_SALT_NULL:
        return QStringLiteral(
            "One of the arguments to the function was NULL when it was not expected.");
    case TOX_ERR_GET_SALT_BAD_FORMAT:
        return QStringLiteral("The input data is missing the magic number or is corrupted.");
    default:
        return QStringLiteral("Unknown salt error.");
    }
}
}
/**
  * @class ToxEncrypt
  * @brief Encapsulates the toxencrypsave API.
  * Since key derivation is work intensive and to avoid storing plaintext
  * passwords in memory, use a ToxEncrypt object and encrypt() or decrypt()
  * when you have to encrypt or decrypt more than once with the same password.
  */

/**
 * @brief Frees the passKey before destruction.
 */
ToxEncrypt::~ToxEncrypt()
{
    tox_pass_key_free(passKey);
}

/**
 * @brief Constructs a ToxEncrypt object from a Tox_Pass_Key.
 * @param key Derived key to use for encryption and decryption.
 */
ToxEncrypt::ToxEncrypt(Tox_Pass_Key* key)
    : passKey{key}
{
}

/**
 * @brief  Gets the minimum number of bytes needed for isEncrypted()
 * @return Minimum number of bytes needed to check if data was encrypted
 *         using this module.
 */
int ToxEncrypt::getMinBytes()
{
    return TOX_PASS_ENCRYPTION_EXTRA_LENGTH;
}

/**
 * @brief Checks if the data was encrypted by this module.
 * @param ciphertext The data to check.
 * @return True if the data was encrypted using this module, false otherwise.
 */
bool ToxEncrypt::isEncrypted(const QByteArray& ciphertext)
{
    if (ciphertext.length() < TOX_PASS_ENCRYPTION_EXTRA_LENGTH) {
        return false;
    }

    return tox_is_data_encrypted(reinterpret_cast<const uint8_t*>(ciphertext.constData()));
}


/**
 * @brief  Encrypts the plaintext with the given password.
 * @return Encrypted data or empty QByteArray on failure.
 * @param  password Password to encrypt the data.
 * @param  plaintext The data to encrypt.
 */
QByteArray ToxEncrypt::encryptPass(const QString& password, const QByteArray& plaintext)
{
    if (password.length() == 0) {
        qWarning() << "Empty password supplied, probably not what you intended.";
    }

    QByteArray pass = password.toUtf8();
    QByteArray ciphertext(plaintext.length() + TOX_PASS_ENCRYPTION_EXTRA_LENGTH, 0x00);
    Tox_Err_Encryption error;
    tox_pass_encrypt(reinterpret_cast<const uint8_t*>(plaintext.constData()),
                     static_cast<size_t>(plaintext.size()),
                     reinterpret_cast<const uint8_t*>(pass.constData()),
                     static_cast<size_t>(pass.size()),
                     reinterpret_cast<uint8_t*>(ciphertext.data()), &error);

    if (error != TOX_ERR_ENCRYPTION_OK) {
        qCritical() << getEncryptionError(error);
        return QByteArray{};
    }

    return ciphertext;
}


/**
 * @brief  Decrypts data encrypted with this module.
 * @return The plaintext or an empty QByteArray on failure.
 * @param  password The password used to encrypt the data.
 * @param  ciphertext The encrypted data.
 */
QByteArray ToxEncrypt::decryptPass(const QString& password, const QByteArray& ciphertext)
{
    if (!isEncrypted(ciphertext)) {
        qWarning() << "The data was not encrypted using this module, or it's corrupted.";
        return QByteArray{};
    }

    if (password.length() == 0) {
        qDebug() << "Empty password supplied, probably not what you intended.";
    }

    QByteArray pass = password.toUtf8();
    QByteArray plaintext(ciphertext.length() - TOX_PASS_ENCRYPTION_EXTRA_LENGTH, 0x00);
    Tox_Err_Decryption error;
    tox_pass_decrypt(reinterpret_cast<const uint8_t*>(ciphertext.constData()),
                     static_cast<size_t>(ciphertext.size()),
                     reinterpret_cast<const uint8_t*>(pass.constData()),
                     static_cast<size_t>(pass.size()), reinterpret_cast<uint8_t*>(plaintext.data()),
                     &error);

    if (error != TOX_ERR_DECRYPTION_OK) {
        qWarning() << getDecryptionError(error);
        return QByteArray{};
    }

    return plaintext;
}

/**
 * @brief  Factory method for the ToxEncrypt object.
 * @param  password Password to use for encryption.
 * @return A std::unique_ptr containing a ToxEncrypt object on success, or an
 *         or an empty std::unique_ptr on failure.
 *
 *  Derives a key from the password and a new random salt.
 */
std::unique_ptr<ToxEncrypt> ToxEncrypt::makeToxEncrypt(const QString& password)
{
    const QByteArray pass = password.toUtf8();
    Tox_Err_Key_Derivation error;
    Tox_Pass_Key* const passKey = tox_pass_key_derive(
        reinterpret_cast<const uint8_t*>(pass.constData()),
        static_cast<size_t>(pass.length()), &error);

    if (error != TOX_ERR_KEY_DERIVATION_OK) {
        tox_pass_key_free(passKey);
        qCritical() << getKeyDerivationError(error);
        return std::unique_ptr<ToxEncrypt>{};
    }

    return std::unique_ptr<ToxEncrypt>(new ToxEncrypt(passKey));
}

/**
 * @brief  Factory method for the ToxEncrypt object.
 * @param  password Password to use for encryption.
 * @param  toxSave The data to read the salt for decryption from.
 * @return A std::unique_ptr containing a ToxEncrypt object on success, or an
 *         or an empty std::unique_ptr on failure.
 *
 *  Derives a key from the password and the salt read from toxSave.
 */
std::unique_ptr<ToxEncrypt> ToxEncrypt::makeToxEncrypt(const QString& password, const QByteArray& toxSave)
{
    if (!isEncrypted(toxSave)) {
        qWarning() << "The data was not encrypted using this module, or it's corrupted.";
        return std::unique_ptr<ToxEncrypt>{};
    }

    Tox_Err_Get_Salt saltError;
    uint8_t salt[TOX_PASS_SALT_LENGTH];
    tox_get_salt(reinterpret_cast<const uint8_t*>(toxSave.constData()), salt, &saltError);

    if (saltError != TOX_ERR_GET_SALT_OK) {
        qWarning() << getSaltError(saltError);
        return std::unique_ptr<ToxEncrypt>{};
    }

    QByteArray pass = password.toUtf8();
    Tox_Err_Key_Derivation keyError;
    Tox_Pass_Key* const passKey = tox_pass_key_derive_with_salt(
        reinterpret_cast<const uint8_t*>(pass.constData()),
        static_cast<size_t>(pass.length()), salt, &keyError);

    if (keyError != TOX_ERR_KEY_DERIVATION_OK) {
        tox_pass_key_free(passKey);
        qWarning() << getKeyDerivationError(keyError);
        return std::unique_ptr<ToxEncrypt>{};
    }

    return std::unique_ptr<ToxEncrypt>(new ToxEncrypt(passKey));
}

/**
 * @brief  Encrypts the plaintext with the stored key.
 * @return Encrypted data or empty QByteArray on failure.
 * @param  plaintext The data to encrypt.
 */
QByteArray ToxEncrypt::encrypt(const QByteArray& plaintext) const
{
    if (!passKey) {
        qCritical() << "The passkey is invalid.";
        return QByteArray{};
    }

    QByteArray ciphertext(plaintext.length() + TOX_PASS_ENCRYPTION_EXTRA_LENGTH, 0x00);
    Tox_Err_Encryption error;
    tox_pass_key_encrypt(passKey, reinterpret_cast<const uint8_t*>(plaintext.constData()),
                         static_cast<size_t>(plaintext.size()),
                         reinterpret_cast<uint8_t*>(ciphertext.data()), &error);

    if (error != TOX_ERR_ENCRYPTION_OK) {
        qCritical() << getEncryptionError(error);
        return QByteArray{};
    }

    return ciphertext;
}


/**
 * @brief  Decrypts data encrypted with this module, using the stored key.
 * @return The plaintext or an empty QByteArray on failure.
 * @param  ciphertext The encrypted data.
 */
QByteArray ToxEncrypt::decrypt(const QByteArray& ciphertext) const
{
    if (!isEncrypted(ciphertext)) {
        qWarning() << "The data was not encrypted using this module, or it's corrupted.";
        return QByteArray{};
    }

    QByteArray plaintext(ciphertext.length() - TOX_PASS_ENCRYPTION_EXTRA_LENGTH, 0x00);
    Tox_Err_Decryption error;
    tox_pass_key_decrypt(passKey, reinterpret_cast<const uint8_t*>(ciphertext.constData()),
                         static_cast<size_t>(ciphertext.size()),
                         reinterpret_cast<uint8_t*>(plaintext.data()), &error);

    if (error != TOX_ERR_DECRYPTION_OK) {
        qWarning() << getDecryptionError(error);
        return QByteArray{};
    }

    return plaintext;
}
