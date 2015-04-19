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
#include "src/widget/gui.h"
#include "src/misc/settings.h"
#include "misc/cstring.h"
#include "historykeeper.h"
#include <tox/tox.h>
#include <tox/toxencryptsave.h>
#include <QApplication>
#include <QDebug>
#include <QSaveFile>
#include <QFile>
#include <QThread>
#include <algorithm>
#include <cassert>

void Core::setPassword(QString& password, PasswordType passtype, uint8_t* salt)
{
    clearPassword(passtype);
    if (password.isEmpty())
        return;

    pwsaltedkeys[passtype] = new TOX_PASS_KEY;

    CString str(password);
    if (salt)
        tox_derive_key_with_salt(str.data(), str.size(), salt, pwsaltedkeys[passtype], nullptr);
    else
        tox_derive_key_from_pass(str.data(), str.size(), pwsaltedkeys[passtype], nullptr);

    password.clear();
}

void Core::useOtherPassword(PasswordType type)
{
    clearPassword(type);
    pwsaltedkeys[type] = new TOX_PASS_KEY;

    PasswordType other = (type == ptMain) ? ptHistory : ptMain;

    std::copy(pwsaltedkeys[other], pwsaltedkeys[other]+TOX_PASS_KEY_LENGTH, pwsaltedkeys[type]);
}

void Core::clearPassword(PasswordType passtype)
{
    delete[] pwsaltedkeys[passtype];
    pwsaltedkeys[passtype] = nullptr;
}

// part of a hack, see core.h
void Core::saveCurrentInformation()
{
    if (pwsaltedkeys[ptMain])
    {
        backupkeys[ptMain] = new TOX_PASS_KEY;
        std::copy(pwsaltedkeys[ptMain], pwsaltedkeys[ptMain]+TOX_PASS_KEY_LENGTH, backupkeys[ptMain]);
    }
    if (pwsaltedkeys[ptHistory])
    {
        backupkeys[ptHistory] = new TOX_PASS_KEY;
        std::copy(pwsaltedkeys[ptHistory], pwsaltedkeys[ptHistory]+TOX_PASS_KEY_LENGTH, backupkeys[ptHistory]);
    }
    backupProfile = new QString(Settings::getInstance().getCurrentProfile());
}

QString Core::loadOldInformation()
{
    QString out;
    if (backupProfile)
    {
        out  = *backupProfile;
        delete backupProfile;
        backupProfile = nullptr;
    }
    backupProfile = nullptr;
    clearPassword(ptMain);
    clearPassword(ptHistory);
    // we can just copy the pointer, as long as we null out backupkeys
    // (if backupkeys was null anyways, then this is a null-op)
    pwsaltedkeys[ptMain]    = backupkeys[ptMain];
    pwsaltedkeys[ptHistory] = backupkeys[ptHistory];
    backupkeys[ptMain]    = nullptr;
    backupkeys[ptHistory] = nullptr;
    return out;
}

QByteArray Core::encryptData(const QByteArray& data, PasswordType passtype)
{
    if (!pwsaltedkeys[passtype])
        return QByteArray();
    uint8_t encrypted[data.size() + TOX_PASS_ENCRYPTION_EXTRA_LENGTH];
    if (!tox_pass_key_encrypt(reinterpret_cast<const uint8_t*>(data.data()), data.size(),
                            pwsaltedkeys[passtype], encrypted, nullptr))
    {
        qWarning() << "Core::encryptData: encryption failed";
        return QByteArray();
    }
    return QByteArray(reinterpret_cast<char*>(encrypted), data.size() + TOX_PASS_ENCRYPTION_EXTRA_LENGTH);
}

QByteArray Core::decryptData(const QByteArray& data, PasswordType passtype)
{
    if (!pwsaltedkeys[passtype])
        return QByteArray();
    int sz = data.size() - TOX_PASS_ENCRYPTION_EXTRA_LENGTH;
    uint8_t decrypted[sz];
    if (!tox_pass_key_decrypt(reinterpret_cast<const uint8_t*>(data.data()), data.size(),
                              pwsaltedkeys[passtype], decrypted, nullptr))
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
        qWarning() << "Core: file" << filename << "doesn't exist";
        return QByteArray();
    }
    QByteArray data = file.read(TOX_PASS_ENCRYPTION_EXTRA_LENGTH);
    file.close();

    uint8_t *salt = new uint8_t[TOX_PASS_SALT_LENGTH];
    int err = tox_get_salt(reinterpret_cast<uint8_t *>(data.data()), salt);
    if (err)
    {
        qWarning() << "Core: can't get salt from" << filename << "header";
        return QByteArray();
    }

    QByteArray res = QByteArray::fromRawData(reinterpret_cast<const char*>(salt), TOX_PASS_SALT_LENGTH);
    delete[] salt;
    return res;
}

bool Core::loadEncryptedSave(QByteArray& data)
{
    assert(0);
    /*
    if (!Settings::getInstance().getEncryptTox())
        GUI::showWarning(tr("Encryption error"), tr("The .tox file is encrypted, but encryption was not checked, continuing regardless."));

    int error = -1;
    QString a(tr("Please enter the password for the %1 profile.", "used in load() when no pw is already set").arg(Settings::getInstance().getCurrentProfile()));
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
        dialogtxt = tr("The profile password failed. Please try another?", "used only when pw set before load() doesn't work");
    }
    else
        dialogtxt = a;

    uint8_t salt[TOX_PASS_SALT_LENGTH];
    tox_get_salt(reinterpret_cast<uint8_t *>(data.data()), salt);

    do
    {
        QString pw = GUI::passwordDialog(tr("Change profile"), dialogtxt);

        if (pw.isEmpty())
        {
            clearPassword(ptMain);
            return false;
        }
        else
            setPassword(pw, ptMain, salt);

        error = tox_encrypted_key_load(tox, reinterpret_cast<uint8_t *>(data.data()), data.size(), pwsaltedkeys[ptMain]);
        dialogtxt = a + "\n" + b;
    } while (error != 0);

    Settings::getInstance().setEncryptTox(true);
    return true;
    */
}

void Core::checkEncryptedHistory()
{
    QString path = HistoryKeeper::getHistoryPath();
    bool exists = QFile::exists(path);

    QByteArray salt = getSaltFromFile(path);
    if (exists && salt.size() == 0)
    {   // maybe we should handle this better
        GUI::showWarning(tr("Encrypted chat history"), tr("No encrypted chat history file found, or it was corrupted.\nHistory will be disabled!"));
        Settings::getInstance().setEncryptLogs(false);
        Settings::getInstance().setEnableLogging(false);
        HistoryKeeper::resetInstance();
        return;
    }

    QString a(tr("Please enter the password for the chat history for the %1 profile.", "used in load() when no hist pw set").arg(Settings::getInstance().getCurrentProfile()));
    QString b(tr("The previous password is incorrect; please try again:", "used on retries in load()"));
    QString c(tr("\nDisabling chat history now will leave the encrypted history intact (but not usable); if you later remember the password, you may re-enable encryption from the Privacy tab with the correct password to use the history.", "part of history password dialog"));
    QString dialogtxt;

    if (pwsaltedkeys[ptHistory])
    {
        if (!exists || HistoryKeeper::checkPassword())
            return;
        dialogtxt = tr("The chat history password failed. Please try another?", "used only when pw set before load() doesn't work");
    }
    else
        dialogtxt = a;
    dialogtxt += "\n" + c;

    if (pwsaltedkeys[ptMain])
    {
        useOtherPassword(ptHistory);
        if (!exists || HistoryKeeper::checkPassword())
        {
            qDebug() << "Core: using main password for chat history";
            return;
        }
        clearPassword(ptHistory);
    }

    bool error = true;
    do
    {
        QString pw = GUI::passwordDialog(tr("Disable chat history"), dialogtxt);

        if (pw.isEmpty())
        {
            clearPassword(ptHistory);
            Settings::getInstance().setEncryptLogs(false);
            Settings::getInstance().setEnableLogging(false);
            HistoryKeeper::resetInstance();
            return;
        }
        else
            setPassword(pw, ptHistory, reinterpret_cast<uint8_t*>(salt.data()));

        error = exists && !HistoryKeeper::checkPassword();
        dialogtxt = a + "\n" + c + "\n" + b;
    } while (error);
}

void Core::saveConfiguration(const QString& path)
{
    if (QThread::currentThread() != coreThread)
        return (void) QMetaObject::invokeMethod(this, "saveConfiguration", Q_ARG(const QString&, path));

    if (!isReady())
    {
        qWarning() << "Core::saveConfiguration: Tox not started, aborting!";
        return;
    }

    QSaveFile configurationFile(path);
    if (!configurationFile.open(QIODevice::WriteOnly)) {
        qCritical() << "File " << path << " cannot be opened";
        return;
    }

    qDebug() << "Core: writing tox_save to " << path;

    uint32_t fileSize;
    bool encrypt = Settings::getInstance().getEncryptTox();
    if (encrypt)
    {
        qCritical() << "Encrypted saving not implemented!";
        exit(-1);
    }
    else
        fileSize = tox_get_savedata_size(tox);

    if (fileSize > 0 && fileSize <= std::numeric_limits<int32_t>::max()) {
        uint8_t *data = new uint8_t[fileSize];

        if (encrypt)
        {
            if (!pwsaltedkeys[ptMain])
            {
                // probably zero chance event
                GUI::showWarning(tr("NO Password"), tr("Local file encryption is enabled, but there is no password! It will be disabled."));
                Settings::getInstance().setEncryptTox(false);
                tox_get_savedata(tox, data);
            }
            else
            {
                qCritical() << "Encryption not implemented";
                exit(-1);
                /*
                int ret = tox_encrypted_key_save(tox, data, pwsaltedkeys[ptMain]);
                if (ret == -1)
                {
                    qCritical() << "Core::saveConfiguration: encryption of save file failed!!!";
                    return;
                }
                */
            }
        }
        else
            tox_get_savedata(tox, data);

        configurationFile.write(reinterpret_cast<char *>(data), fileSize);
        configurationFile.commit();
        delete[] data;
    }

    Settings::getInstance().save();
}
