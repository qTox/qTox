/*
    Copyright © 2014-2015 by The qTox Project

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
#include "src/widget/gui.h"
#include "src/persistence/settings.h"
#include "src/core/cstring.h"
#include "src/persistence/historykeeper.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include <tox/tox.h>
#include <tox/toxencryptsave.h>
#include <QApplication>
#include <QDebug>
#include <QSaveFile>
#include <QFile>
#include <QThread>
#include <algorithm>
#include <cassert>

std::unique_ptr<TOX_PASS_KEY> Core::createPasskey(const QString& password, uint8_t* salt)
{
    std::unique_ptr<TOX_PASS_KEY> encryptionKey(new TOX_PASS_KEY);

    CString str(password);
    if (salt)
        tox_derive_key_with_salt(str.data(), str.size(), salt, encryptionKey.get(), nullptr);
    else
        tox_derive_key_from_pass(str.data(), str.size(), encryptionKey.get(), nullptr);

    return encryptionKey;
}

QByteArray Core::encryptData(const QByteArray &data)
{
    return encryptData(data, Nexus::getProfile()->getPasskey());
}

QByteArray Core::encryptData(const QByteArray& data, const TOX_PASS_KEY& encryptionKey)
{
    uint8_t encrypted[data.size() + TOX_PASS_ENCRYPTION_EXTRA_LENGTH];
    if (!tox_pass_key_encrypt(reinterpret_cast<const uint8_t*>(data.data()), data.size(),
                            &encryptionKey, encrypted, nullptr))
    {
        qWarning() << "Encryption failed";
        return QByteArray();
    }
    return QByteArray(reinterpret_cast<char*>(encrypted), data.size() + TOX_PASS_ENCRYPTION_EXTRA_LENGTH);
}

QByteArray Core::decryptData(const QByteArray &data)
{
    return decryptData(data, Nexus::getProfile()->getPasskey());
}

QByteArray Core::decryptData(const QByteArray& data, const TOX_PASS_KEY& encryptionKey)
{
    int sz = data.size() - TOX_PASS_ENCRYPTION_EXTRA_LENGTH;
    uint8_t decrypted[sz];
    if (!tox_pass_key_decrypt(reinterpret_cast<const uint8_t*>(data.data()), data.size(),
                              &encryptionKey, decrypted, nullptr))
    {
        qWarning() << "Decryption failed";
        return QByteArray();
    }
    return QByteArray(reinterpret_cast<char*>(decrypted), sz);
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

    uint8_t *salt = new uint8_t[TOX_PASS_SALT_LENGTH];
    if (!tox_get_salt(reinterpret_cast<uint8_t *>(data.data()), salt))
    {
        qWarning() << "can't get salt from" << filename << "header";
        return QByteArray();
    }

    QByteArray res(reinterpret_cast<const char*>(salt), TOX_PASS_SALT_LENGTH);
    delete[] salt;
    return res;
}

void Core::checkEncryptedHistory()
{
    QString path = HistoryKeeper::getHistoryPath();
    bool exists = QFile::exists(path) && QFile(path).size()>0;

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


    if (!exists || HistoryKeeper::checkPassword(*passkey))
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

        error = exists && !HistoryKeeper::checkPassword(*passkey);
        dialogtxt = a + "\n" + c + "\n" + b;
    } while (error);
}
