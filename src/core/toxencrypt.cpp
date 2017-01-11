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

#include "toxencrypt.h"
#include <tox/toxencryptsave.h>

#include <QByteArray>
#include <QDebug>
#include <QString>

// functions for nice debug output
static QString getKeyDerivationError(TOX_ERR_KEY_DERIVATION error);
static QString getEncryptionError(TOX_ERR_ENCRYPTION error);
static QString getDecryptionError(TOX_ERR_DECRYPTION error);
static QString getSaltError(TOX_ERR_GET_SALT error);

/**
  * @class ToxEncrypt
  * @brief Encapsulates the toxencrypsave API.
  * Since key derivation is work intensive, use a ToxEncrypt object and
  * encrypt() or decrypt() when you have to encrypt or decrypt more than once
  * with the same password.
  */

/**
 * @brief Frees the passKey before destruction.
 */
ToxEncrypt::~ToxEncrypt()
{
    tox_pass_key_free(passKey);
}

/**
 * @brief  Creates a new ToxEncrypt object, with a random salt.
 * @param  password Password to use for encryption.
 */
ToxEncrypt::ToxEncrypt(const QString& password)
{
    passKey = tox_pass_key_new();
    QByteArray pass = password.toUtf8();
    TOX_ERR_KEY_DERIVATION error;
    tox_pass_key_derive(passKey, reinterpret_cast<const uint8_t*>(pass.constData()),
                        static_cast<size_t>(pass.length()), &error);

    if(error != TOX_ERR_KEY_DERIVATION_OK)
    {
        qWarning() << getKeyDerivationError(error);
        tox_pass_key_free(passKey);
        passKey = nullptr;
    }
}


/**
 * @brief  Creates a new ToxEncrypt object, with the salt read from the toxSave data.
 * @param  password The Password for the encrypted data.
 * @param  toxSave The data to read the salt from.
 */
ToxEncrypt::ToxEncrypt(const QString& password, const QByteArray& toxSave)
{
    if(!isEncrypted(toxSave))
    {
        qWarning() << "The data was not encrypted using this module or it's corrupted.";
        return;
    }

    TOX_ERR_GET_SALT saltError;
    uint8_t salt[TOX_PASS_SALT_LENGTH];
    tox_get_salt(reinterpret_cast<const uint8_t*>(toxSave.constData()), salt, &saltError);

    if(saltError != TOX_ERR_GET_SALT_OK)
    {
        qWarning() << getSaltError(saltError);
        return;
    }

    passKey = tox_pass_key_new();
    QByteArray pass = password.toUtf8();
    TOX_ERR_KEY_DERIVATION keyError;
    tox_pass_key_derive_with_salt(passKey, reinterpret_cast<const uint8_t*>(pass.constData()),
                        static_cast<size_t>(pass.length()), salt, &keyError);

    if(keyError != TOX_ERR_KEY_DERIVATION_OK)
    {
        qWarning() << getKeyDerivationError(keyError);
        tox_pass_key_free(passKey);
        passKey = nullptr;
    }
}

/**
 * @brief Checks if the data was encrypted by this module.
 * @param ciphertext The data to check.
 * @return True if the data was encrypted using this module, false otherwise.
 */
bool ToxEncrypt::isEncrypted(const QByteArray& ciphertext)
{
    if(ciphertext.length() < TOX_PASS_ENCRYPTION_EXTRA_LENGTH)
    {
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
    if(password.length() == 0)
    {
        qWarning() << "Empty password supplied, probably not what you intended.";
    }

    QByteArray pass = password.toUtf8();
    QByteArray ciphertext(plaintext.length() + TOX_PASS_ENCRYPTION_EXTRA_LENGTH, 0x00);
    TOX_ERR_ENCRYPTION error;
    tox_pass_encrypt(reinterpret_cast<const uint8_t*>(plaintext.constData()),
                     static_cast<size_t>(plaintext.size()),
                     reinterpret_cast<const uint8_t*>(pass.constData()),
                     static_cast<size_t>(pass.size()),
                     reinterpret_cast<uint8_t*>(ciphertext.data()), &error);

    if(error != TOX_ERR_ENCRYPTION_OK)
    {
        qWarning() << getEncryptionError(error);
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
    if(!isEncrypted(ciphertext))
    {
        qWarning() << "The data was not encrypted using this module or it's corrupted.";
        return QByteArray{};
    }

    if(password.length() == 0)
    {
        qDebug() << "Empty password supplied, probably not what you intended.";
    }

    QByteArray pass = password.toUtf8();
    QByteArray plaintext(ciphertext.length() - TOX_PASS_ENCRYPTION_EXTRA_LENGTH, 0x00);
    TOX_ERR_DECRYPTION error;
    tox_pass_decrypt(reinterpret_cast<const uint8_t*>(ciphertext.constData()),
                     static_cast<size_t>(ciphertext.size()),
                     reinterpret_cast<const uint8_t*>(pass.constData()),
                     static_cast<size_t>(pass.size()),
                     reinterpret_cast<uint8_t*>(plaintext.data()), &error);

    if(error != TOX_ERR_DECRYPTION_OK)
    {
        qWarning() << getDecryptionError(error);
        return QByteArray{};
    }

    return ciphertext;
}

/**
 * @brief  Checks if the object can be used for encryption and decryption.
 * @return True if encryption and decryption is possible, false otherwise.
 */
bool ToxEncrypt::isValid() const
{
    return passKey != nullptr;
}

/**
 * @brief  Encrypts the plaintext with the stored key.
 * @return Encrypted data or empty QByteArray on failure.
 * @param  plaintext The data to encrypt.
 */
QByteArray ToxEncrypt::encrypt(const QByteArray& plaintext) const
{
    if(!passKey)
    {
        qWarning() << "The passKey is invalid.";
        return QByteArray{};
    }

    QByteArray ciphertext(plaintext.length() + TOX_PASS_ENCRYPTION_EXTRA_LENGTH, 0x00);
    TOX_ERR_ENCRYPTION error;
    tox_pass_key_encrypt(passKey,
                         reinterpret_cast<const uint8_t*>(plaintext.constData()),
                         static_cast<size_t>(plaintext.size()),
                         reinterpret_cast<uint8_t*>(ciphertext.data()), &error);

    if(error != TOX_ERR_ENCRYPTION_OK)
    {
        qWarning() << getEncryptionError(error);
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
    if(!isEncrypted(ciphertext))
    {
        qWarning() << "The data was not encrypted using this module or it's corrupted.";
        return QByteArray{};
    }

    QByteArray plaintext(ciphertext.length() - TOX_PASS_ENCRYPTION_EXTRA_LENGTH, 0x00);
    TOX_ERR_DECRYPTION error;
    tox_pass_key_decrypt(passKey,
                         reinterpret_cast<const uint8_t*>(ciphertext.constData()),
                         static_cast<size_t>(ciphertext.size()),
                         reinterpret_cast<uint8_t*>(plaintext.data()), &error);

    if(error != TOX_ERR_DECRYPTION_OK)
    {
        qWarning() << getDecryptionError(error);
        return QByteArray{};
    }

    return ciphertext;
}
/**
 * @brief Gets the error string for TOX_ERR_KEY_DERIVATION errors.
 * @param error The error number.
 * @return The verbose error message.
 */
QString getKeyDerivationError(TOX_ERR_KEY_DERIVATION error)
{
    switch(error)
    {
    case TOX_ERR_KEY_DERIVATION_OK:
        return QStringLiteral("The function returned successfully.");
    case TOX_ERR_KEY_DERIVATION_NULL:
        return QStringLiteral("One of the arguments to the function was NULL when it was not expected.");
    case TOX_ERR_KEY_DERIVATION_FAILED:
        return QStringLiteral("The crypto lib was unable to derive a key from the given passphrase.");
    default:
        return QStringLiteral("Unknown key derivation error.");
    }
}

/**
 * @brief Gets the error string for TOX_ERR_ENCRYPTION errors.
 * @param error The error number.
 * @return The verbose error message.
 */
QString getEncryptionError(TOX_ERR_ENCRYPTION error)
{
    switch(error)
    {
    case TOX_ERR_ENCRYPTION_OK:
        return QStringLiteral("The function returned successfully.");
    case TOX_ERR_ENCRYPTION_NULL:
        return QStringLiteral("One of the arguments to the function was NULL when it was not expected.");
    case TOX_ERR_ENCRYPTION_KEY_DERIVATION_FAILED:
        return QStringLiteral("The crypto lib was unable to derive a key from the given passphrase.");
    case TOX_ERR_ENCRYPTION_FAILED:
        return QStringLiteral("The encryption itself failed.");
    default:
        return QStringLiteral("Unknown encryption error.");
    }
}

/**
 * @brief Gets the error string for TOX_ERR_DECRYPTION errors.
 * @param error The error number.
 * @return The verbose error message.
 */
QString getDecryptionError(TOX_ERR_DECRYPTION error)
{
    switch(error)
    {
    case TOX_ERR_DECRYPTION_OK:
        return QStringLiteral("The function returned successfully.");
    case TOX_ERR_DECRYPTION_NULL:
        return QStringLiteral("One of the arguments to the function was NULL when it was not expected.");
    case TOX_ERR_DECRYPTION_INVALID_LENGTH:
        return QStringLiteral("The input data was shorter than TOX_PASS_ENCRYPTION_EXTRA_LENGTH bytes.");
    case TOX_ERR_DECRYPTION_BAD_FORMAT:
        return QStringLiteral("The input data is missing the magic number or is corrupted.");
    default:
        return QStringLiteral("Unknown decryption error.");
    }
}

/**
 * @brief Gets the error string for TOX_ERR_GET_SALT errors.
 * @param error The error number.
 * @return The verbose error message.
 */
QString getSaltError(TOX_ERR_GET_SALT error)
{
    switch(error)
    {
    case TOX_ERR_GET_SALT_OK:
        return QStringLiteral("The function returned successfully.");
    case TOX_ERR_GET_SALT_NULL:
        return QStringLiteral("One of the arguments to the function was NULL when it was not expected.");
    case TOX_ERR_GET_SALT_BAD_FORMAT:
        return QStringLiteral("The input data is missing the magic number or is corrupted.");
    default:
        return QStringLiteral("Unknown salt error.");
    }
}
