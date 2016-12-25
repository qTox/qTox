/*
    Copyright © 2014-2015 by The qTox Project Contributors

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

/* This file is part of the Core class, but was separated for my sanity */
/* The load function delegates to loadEncrypted here, and the save function */
/* was permanently moved here to handle encryption */

#include "core.h"
#include "widget/gui.h"
#include "persistence/settings.h"
#include "core/cstring.h"
#include "nexus.h"
#include "persistence/profile.h"
#include "persistence/historykeeper.h"
#include <tox/tox.h>
#include <tox/toxencryptsave.h>
#include <QApplication>
#include <QDebug>
#include <QSaveFile>
#include <QFile>
#include <QThread>
#include <algorithm>
#include <cassert>

std::shared_ptr<Tox_Pass_Key> Core::createPasskey(const QString& password, uint8_t* salt)
{
    std::shared_ptr<Tox_Pass_Key> encryptionKey(tox_pass_key_new(), tox_pass_key_free);

    CString str(password);
    if (salt)
        tox_pass_key_derive_with_salt(encryptionKey.get(), str.data(), str.size(), salt, nullptr);
    else
        tox_pass_key_derive(encryptionKey.get(), str.data(), str.size(), nullptr);

    return encryptionKey;
}

/**
 * @brief Encrypts data.
 * @note Uses the default profile's key.
 * @param data Data to encrypt.
 * @return Encrypted data.
 */
QByteArray Core::encryptData(const QByteArray &data)
{
    return encryptData(data, Nexus::getProfile()->getPasskey());
}

QByteArray Core::encryptData(const QByteArray& data, const Tox_Pass_Key& encryptionKey)
{
    QByteArray encrypted = QByteArray(data.size() + TOX_PASS_ENCRYPTION_EXTRA_LENGTH, 0x00);
    if (!tox_pass_key_encrypt(&encryptionKey, reinterpret_cast<const uint8_t*>(data.data()), data.size(),
                              (uint8_t*) encrypted.data(), nullptr))
    {
        qWarning() << "Encryption failed";
        return QByteArray();
    }
    return encrypted;
}

/**
 * @brief Decrypts data.
 * @note Uses the default profile's key.
 * @param data Data to decrypt.
 * @return Decrypted data.
 */
QByteArray Core::decryptData(const QByteArray &data)
{
    return decryptData(data, Nexus::getProfile()->getPasskey());
}

QByteArray Core::decryptData(const QByteArray& data, const Tox_Pass_Key& encryptionKey)
{
    if (data.size() < TOX_PASS_ENCRYPTION_EXTRA_LENGTH)
    {
        qWarning() << "Not enough data:" << data.size();
        return QByteArray();
    }
    int decryptedSize = data.size() - TOX_PASS_ENCRYPTION_EXTRA_LENGTH;
    QByteArray decrypted = QByteArray(decryptedSize, 0x00);
    if (!tox_pass_key_decrypt(&encryptionKey, reinterpret_cast<const uint8_t*>(data.data()), data.size(),
                              (uint8_t*) decrypted.data(), nullptr))
    {
        qWarning() << "Decryption failed";
        return QByteArray();
    }
    return decrypted;
}

QByteArray Core::getSaltFromFile(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "file" << filename << "doesn't exist";
        return QByteArray();
    }
    QByteArray data = file.read(TOX_PASS_ENCRYPTION_EXTRA_LENGTH);
    file.close();

    uint8_t salt[TOX_PASS_SALT_LENGTH];
    if (!tox_get_salt(reinterpret_cast<uint8_t *>(data.data()), salt, nullptr))
    {
        qWarning() << "can't get salt from" << filename << "header";
        return QByteArray();
    }

    QByteArray res(reinterpret_cast<const char*>(salt), TOX_PASS_SALT_LENGTH);
    return res;
}

void Core::checkEncryptedHistory()
{
    QString path = HistoryKeeper::getHistoryPath();
    bool exists = QFile::exists(path) && QFile(path).size()>0;
    if (!exists)
        return;

    QByteArray salt = getSaltFromFile(path);
    if (exists && salt.size() == 0)
    {   // maybe we should handle this better
        GUI::showWarning(tr("Encrypted chat history"), tr("No encrypted chat history file found, or it was corrupted.\nHistory will be disabled!"));
        HistoryKeeper::resetInstance();
        return;
    }

    auto passkey = createPasskey(Nexus::getProfile()->getPassword(), reinterpret_cast<uint8_t*>(salt.data()));

    QString a(tr("Please enter the password for the chat history for the profile \"%1\".", "used in load() when no hist pw set").arg(Nexus::getProfile()->getName()));
    QString b(tr("The previous password is incorrect; please try again:", "used on retries in load()"));
    QString c(tr("\nDisabling chat history now will leave the encrypted history intact (but not usable); if you later remember the password, you may re-enable encryption from the Privacy tab with the correct password to use the history.", "part of history password dialog"));
    QString dialogtxt;


    if (!exists || HistoryKeeper::checkPassword(passkey))
        return;

    dialogtxt = tr("The chat history password failed. Please try another?", "used only when pw set before load() doesn't work");
    dialogtxt += "\n" + c;

    bool error = true;
    do
    {
        QString pw = GUI::passwordDialog(tr("Disable chat history"), dialogtxt);

        if (pw.isEmpty())
        {
            Settings::getInstance().setEnableLogging(false);
            return;
        }
        else
        {
            passkey = createPasskey(pw, reinterpret_cast<uint8_t*>(salt.data()));
        }

        error = exists && !HistoryKeeper::checkPassword(passkey);
        dialogtxt = a + "\n" + c + "\n" + b;
    } while (error);
}
