/*
    Copyright (C) 2013 by Maxim Biro <nurupo.contributions@gmail.com>

    This file is part of Tox Qt GUI.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

/* This file is part of the Core class, but was separated for my sanity */
/* At least temporarily, the save and load functions are here as well */

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
#include <QMessageBox>

bool Core::loadConfiguration(QString path)
{
    // setting the profile is now the responsibility of the caller
    QFile configurationFile(path);
    qDebug() << "Core::loadConfiguration: reading from " << path;

    if (!configurationFile.exists()) {
        qWarning() << "The Tox configuration file was not found";
        return true;
    }

    if (!configurationFile.open(QIODevice::ReadOnly)) {
        qCritical() << "File " << path << " cannot be opened";
        return true;
    }

    qint64 fileSize = configurationFile.size();
    if (fileSize > 0) {
        QByteArray data = configurationFile.readAll();
        int error = tox_load(tox, reinterpret_cast<uint8_t *>(data.data()), data.size());
        if (error < 0)
        {
            qWarning() << "Core: tox_load failed with error "<< error;
        }
        else if (error == 1) // Encrypted data save
        {
            if (!Settings::getInstance().getEncryptTox())
                Widget::getInstance()->showWarningMsgBox(tr("Encryption error"), tr("The .tox file is encrypted, but encryption was not checked, continuing regardless."));
            uint8_t salt[tox_pass_salt_length()];
            tox_get_salt(reinterpret_cast<uint8_t *>(data.data()), salt);
            do
            {
                while (!pwsaltedkeys[ptMain])
                {
                    emit blockingGetPassword(tr("Tox datafile decryption password"), ptMain, salt);
                    if (!pwsaltedkeys[ptMain])
                        Widget::getInstance()->showWarningMsgBox(tr("Password error"), tr("Failed to setup password.\nEmpty password."));
                }

                error = tox_encrypted_key_load(tox, reinterpret_cast<uint8_t *>(data.data()), data.size(), pwsaltedkeys[ptMain]);
                if (error != 0)
                {
                    QMessageBox msgb;
                    msgb.moveToThread(qApp->thread());
                    QPushButton *tryAgain = msgb.addButton(tr("Try Again"), QMessageBox::AcceptRole);
                    QPushButton *cancel = msgb.addButton(tr("Change profile"), QMessageBox::RejectRole);
                    QPushButton *wipe = msgb.addButton(tr("Reinit current profile"), QMessageBox::ActionRole);
                    msgb.setDefaultButton(tryAgain);
                    msgb.setWindowTitle(tr("Password error"));
                    msgb.setText(tr("Wrong password has been entered"));
                    // msgb.setInformativeText(tr(""));

                    msgb.exec();

                    if (msgb.clickedButton() == tryAgain)
                        clearPassword(ptMain);
                    else if (msgb.clickedButton() == cancel)
                    {
                        configurationFile.close();
                        return false;
                    }
                    else if (msgb.clickedButton() == wipe)
                    {
                        clearPassword(ptMain);
                        Settings::getInstance().setEncryptTox(false);
                        error = 0;
                    }
                }
                else
                    Settings::getInstance().setEncryptTox(true);
            } while (error != 0);
        }
    }
    configurationFile.close();

    // set GUI with user and statusmsg
    QString name = getUsername();
    if (!name.isEmpty())
        emit usernameSet(name);
    
    QString msg = getStatusMessage();
    if (!msg.isEmpty())
        emit statusMessageSet(msg);

    QString id = getSelfId().toString();
    if (!id.isEmpty())
        emit idSet(id);

    // tox core is already decrypted
    if (Settings::getInstance().getEnableLogging() && Settings::getInstance().getEncryptLogs())
    {
        bool error = true;
        
        // get salt
        QByteArray salt = getSaltFromFile(HistoryKeeper::getHistoryPath());

        if (salt.size() == 0)
        {   // maybe we should handle this better
            Widget::getInstance()->showWarningMsgBox(tr("Encrypted History"), tr("No encrypted history file found.\nHistory will be disabled!"));
            Settings::getInstance().setEncryptLogs(false);
            Settings::getInstance().setEnableLogging(false);
        }
        else
        {
            do
            {
                while (!pwsaltedkeys[ptHistory])
                {
                    emit blockingGetPassword(tr("History Log decryption password"), Core::ptHistory,
                                             reinterpret_cast<uint8_t*>(salt.data()));
                    if (!pwsaltedkeys[ptHistory])
                        Widget::getInstance()->showWarningMsgBox(tr("Password error"), tr("Failed to setup password.\nEmpty password."));
                }

                if (!HistoryKeeper::checkPassword())
                {
                    if (QMessageBox::Ok == Widget::getInstance()->showWarningMsgBox(tr("Encrypted log"),
                                                                tr("Your history is encrypted with different password\nDo you want to try another password?"),
                                                                QMessageBox::Ok | QMessageBox::Cancel))
                    {
                        error = true;
                        clearPassword(ptHistory);
                    }
                    else
                    {
                        error = false;
                        clearPassword(ptHistory);
                        Widget::getInstance()->showWarningMsgBox(tr("Loggin"), tr("Due to incorret password history will be disabled"));
                        Settings::getInstance().setEncryptLogs(false);
                        Settings::getInstance().setEnableLogging(false);
                    }
                } else {
                    error = false;
                }
            } while (error);
        }
    }

    loadFriends();
    return true;
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
                Widget::getInstance()->showWarningMsgBox(tr("NO Password"), tr("Will be saved without encryption!"));
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

void Core::setPassword(QString& password, PasswordType passtype, uint8_t* salt)
{
    if (password.isEmpty())
    {
        clearPassword(passtype);
        return;
    }
    if (!pwsaltedkeys[passtype])
        pwsaltedkeys[passtype] = new uint8_t[tox_pass_key_length()];

    CString str(password);
    if (salt)
        tox_derive_key_with_salt(str.data(), str.size(), salt, pwsaltedkeys[passtype]);
    else
        tox_derive_key_from_pass(str.data(), str.size(), pwsaltedkeys[passtype]);

    password.clear();
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
    qDebug() << filename;
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.read(tox_pass_encryption_extra_length());
    file.close();

    qDebug() << "data size" << data.size();

    uint8_t *salt = new uint8_t[tox_pass_salt_length()];
    int err = tox_get_salt(reinterpret_cast<uint8_t *>(data.data()), salt);
    if (err)
    {
        qWarning() << "Core: can't get salt from" << filename << "header";
        return QByteArray();
    }

    QByteArray res = QByteArray::fromRawData(reinterpret_cast<const char*>(salt), tox_pass_salt_length());
    qDebug() << res.toBase64();
    return res;
}
