/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

/* This file is part of the Core class, but was separated for my sanity */
/* The load function delegates to loadEncrypted here, and the save function */
/* was permanently moved here to handle encryption */

#include "core.h"
#include "src/widget/widget.h"
#include <tox/tox.h>
#include <tox/toxencryptsave.h>
#include "src/misc/settings.h"
#include "misc/cstring.h"
#include "historykeeper.h"
#include <QApplication>

#include <QDebug>
#include <QSaveFile>
#include <QFile>

void Core::setPassword(QString& password, PasswordType passtype, uint8_t* salt)
{
    clearPassword(passtype);
    if (password.isEmpty())
        return;

    pwsaltedkeys[passtype] = new uint8_t[tox_pass_key_length()];

    CString str(password);
    if (salt)
        tox_derive_key_with_salt(str.data(), str.size(), salt, pwsaltedkeys[passtype]);
    else
        tox_derive_key_from_pass(str.data(), str.size(), pwsaltedkeys[passtype]);

    password.clear();
}

void Core::useOtherPassword(PasswordType type)
{
    clearPassword(type);
    if (type == ptHistory)
        pwsaltedkeys[ptHistory] = pwsaltedkeys[ptMain];
    else if (type == ptMain)
        pwsaltedkeys[ptMain] = pwsaltedkeys[ptHistory];
}

void Core::clearPassword(PasswordType passtype)
{
    if (pwsaltedkeys[passtype])
    {
        delete[] pwsaltedkeys[passtype];
        pwsaltedkeys[passtype] = nullptr;
    }
}

QByteArray Core::encryptData(const QByteArray& data, PasswordType passtype)
{
    if (!pwsaltedkeys[passtype])
        return QByteArray();
    uint8_t encrypted[data.size() + tox_pass_encryption_extra_length()];
    if (tox_pass_key_encrypt(reinterpret_cast<const uint8_t*>(data.data()), data.size(), pwsaltedkeys[passtype], encrypted) == -1)
    {
        qWarning() << "Core::encryptData: encryption failed";
        return QByteArray();
    }
    return QByteArray(reinterpret_cast<char*>(encrypted), data.size() + tox_pass_encryption_extra_length());
}

QByteArray Core::decryptData(const QByteArray& data, PasswordType passtype)
{
    if (!pwsaltedkeys[passtype])
        return QByteArray();
    int sz = data.size() - tox_pass_encryption_extra_length();
    uint8_t decrypted[sz];
    int decr_size = tox_pass_key_decrypt(reinterpret_cast<const uint8_t*>(data.data()), data.size(), pwsaltedkeys[passtype], decrypted);
    if (decr_size != sz)
    {
        qWarning() << "Core::decryptData: decryption failed";
        return QByteArray();
    }
    return QByteArray(reinterpret_cast<char*>(decrypted), sz);
}

bool Core::isPasswordSet(PasswordType passtype)
{
    if (pwsaltedkeys[passtype])
        return true;

    return false;
}

QByteArray Core::getSaltFromFile(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Core: encrypted history file doesn't exist";
        return QByteArray();
    }
    QByteArray data = file.read(tox_pass_encryption_extra_length());
    file.close();

    uint8_t *salt = new uint8_t[tox_pass_salt_length()];
    int err = tox_get_salt(reinterpret_cast<uint8_t *>(data.data()), salt);
    if (err)
    {
        qWarning() << "Core: can't get salt from" << filename << "header";
        return QByteArray();
    }

    QByteArray res = QByteArray::fromRawData(reinterpret_cast<const char*>(salt), tox_pass_salt_length());
    return res;
}

bool Core::loadEncryptedSave(QByteArray& data)
{
    if (!Settings::getInstance().getEncryptTox())
        Widget::getInstance()->showWarningMsgBox(tr("Encryption error"), tr("The .tox file is encrypted, but encryption was not checked, continuing regardless."));

    int error = -1;
    QString a(tr("Please enter the password for this profile.", "used in load() when no pw is already set"));
    QString b(tr("The previous password is incorrect; please try again:", "used on retries in load()"));
    QString dialogtxt;

    if (pwsaltedkeys[ptMain]) // password set, try it
    {
        error = tox_encrypted_key_load(tox, reinterpret_cast<uint8_t *>(data.data()), data.size(), pwsaltedkeys[ptMain]);
        if (!error)
        {
            Settings::getInstance().setEncryptTox(true);
            return true;
        }
        dialogtxt = tr("The stored profile password failed. Please try another:", "used only when pw set before load() doesn't work");
    }
    else
        dialogtxt = a;

    uint8_t salt[tox_pass_salt_length()];
    tox_get_salt(reinterpret_cast<uint8_t *>(data.data()), salt);

    do
    {
        QString pw = Widget::getInstance()->passwordDialog(tr("Change profile"), dialogtxt);

        if (pw.isEmpty())
        {
            clearPassword(ptMain);
            return false;
        }
        else
            setPassword(pw, ptMain, salt);

        error = tox_encrypted_key_load(tox, reinterpret_cast<uint8_t *>(data.data()), data.size(), pwsaltedkeys[ptMain]);
        dialogtxt = a + " " + b;
    } while (error != 0);

    Settings::getInstance().setEncryptTox(true);
    return true;
}

void Core::checkEncryptedHistory()
{
    QByteArray salt = getSaltFromFile(HistoryKeeper::getHistoryPath());

    if (salt.size() == 0)
    {   // maybe we should handle this better
        Widget::getInstance()->showWarningMsgBox(tr("Encrypted History"), tr("No encrypted history file found, or it was corrupted.\nHistory will be disabled!"));
        Settings::getInstance().setEncryptLogs(false);
        Settings::getInstance().setEnableLogging(false);
        return;
    }

    QString a(tr("Please enter the password for the chat logs.", "used in load() when no hist pw set"));
    QString b(tr("The previous password is incorrect; please try again:", "used on retries in load()"));
    QString dialogtxt;

    if (pwsaltedkeys[ptHistory])
    {
        if (HistoryKeeper::checkPassword())
            return;
        dialogtxt = tr("The stored chat log password failed. Please try another:", "used only when pw set before load() doesn't work");
    }
    else
        dialogtxt = a;

    bool error = true;
    do
    {
        QString pw = Widget::getInstance()->passwordDialog(tr("Disable history"), dialogtxt);

        if (pw.isEmpty())
        {
            clearPassword(ptHistory);
            Settings::getInstance().setEncryptLogs(false);
            Settings::getInstance().setEnableLogging(false);
            return;
        }
        else
            setPassword(pw, ptHistory, reinterpret_cast<uint8_t*>(salt.data()));

        error = !HistoryKeeper::checkPassword();
        dialogtxt = a + " " + b;
    } while (error);
}

void Core::saveConfiguration(const QString& path)
{
    if (!tox)
    {
        qWarning() << "Core::saveConfiguration: Tox not started, aborting!";
        return;
    }

    Settings::getInstance().save();

    QSaveFile configurationFile(path);
    if (!configurationFile.open(QIODevice::WriteOnly)) {
        qCritical() << "File " << path << " cannot be opened";
        return;
    }

    qDebug() << "Core: writing tox_save to " << path;

    uint32_t fileSize; bool encrypt = Settings::getInstance().getEncryptTox();
    if (encrypt)
        fileSize = tox_encrypted_size(tox);
    else
        fileSize = tox_size(tox);

    if (fileSize > 0 && fileSize <= INT32_MAX) {
        uint8_t *data = new uint8_t[fileSize];

        if (encrypt)
        {
            if (!pwsaltedkeys[ptMain])
            {
                // probably zero chance event
                Widget::getInstance()->showWarningMsgBox(tr("NO Password"), tr("Encryption is enabled, but there is no password! Encryption will be disabled."));
                Settings::getInstance().setEncryptTox(false);
                tox_save(tox, data);
            }
            else
            {
                int ret = tox_encrypted_key_save(tox, data, pwsaltedkeys[ptMain]);
                if (ret == -1)
                {
                    qCritical() << "Core::saveConfiguration: encryption of save file failed!!!";
                    return;
                }
            }
        }
        else
            tox_save(tox, data);

        configurationFile.write(reinterpret_cast<char *>(data), fileSize);
        configurationFile.commit();
        delete[] data;
    }
}
