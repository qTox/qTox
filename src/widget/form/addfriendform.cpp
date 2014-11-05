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

#include "addfriendform.h"

#include <QFont>
#include <QMessageBox>
#include <QThread>
#include <tox/tox.h>
#include "ui_mainwindow.h"
#include "src/core.h"
#include "src/misc/cdata.h"
#include "tox/toxdns.h"

#include <QDebug>

#define TOX_ID_LENGTH 2*TOX_FRIEND_ADDRESS_SIZE

const AddFriendForm::tox3_server AddFriendForm::pinnedServers[]
{
    {"toxme.se", (uint8_t[32]){0x5D, 0x72, 0xC5, 0x17, 0xDF, 0x6A, 0xEC, 0x54, 0xF1, 0xE9, 0x77, 0xA6, 0xB6, 0xF2, 0x59, 0x14,
                0xEA, 0x4C, 0xF7, 0x27, 0x7A, 0x85, 0x02, 0x7C, 0xD9, 0xF5, 0x19, 0x6D, 0xF1, 0x7E, 0x0B, 0x13}},
    {"utox.org", (uint8_t[32]){0xD3, 0x15, 0x4F, 0x65, 0xD2, 0x8A, 0x5B, 0x41, 0xA0, 0x5D, 0x4A, 0xC7, 0xE4, 0xB3, 0x9C, 0x6B,
                  0x1C, 0x23, 0x3C, 0xC8, 0x57, 0xFB, 0x36, 0x5C, 0x56, 0xE8, 0x39, 0x27, 0x37, 0x46, 0x2A, 0x12}}
};

AddFriendForm::AddFriendForm() : dns(this)
{
    dns.setType(QDnsLookup::TXT);

    main = new QWidget(), head = new QWidget();
    QFont bold;
    bold.setBold(true);
    headLabel.setText(tr("Add Friends"));
    headLabel.setFont(bold);

    toxIdLabel.setText(tr("Tox ID","Tox ID of the person you're sending a friend request to"));
    messageLabel.setText(tr("Message","The message you send in friend requests"));
    sendButton.setText(tr("Send friend request"));
    message.setPlaceholderText(tr("Tox me maybe?","Default message in friend requests if the field is left blank. Write something appropriate!"));

    main->setLayout(&layout);
    layout.addWidget(&toxIdLabel);
    layout.addWidget(&toxId);
    layout.addWidget(&messageLabel);
    layout.addWidget(&message);
    layout.addWidget(&sendButton);

    head->setLayout(&headLayout);
    headLayout.addWidget(&headLabel);

    connect(&sendButton, SIGNAL(clicked()), this, SLOT(onSendTriggered()));
}

AddFriendForm::~AddFriendForm()
{
    head->deleteLater();
    main->deleteLater();
}

void AddFriendForm::show(Ui::MainWindow &ui)
{
    ui.mainContent->layout()->addWidget(main);
    ui.mainHead->layout()->addWidget(head);
    main->show();
    head->show();
}

bool AddFriendForm::isToxId(const QString &value) const
{
    const QRegularExpression hexRegExp("^[A-Fa-f0-9]+$");
    return value.length() == TOX_ID_LENGTH && value.contains(hexRegExp);
}

void AddFriendForm::showWarning(const QString &message) const
{
    QMessageBox warning(main);
    warning.setWindowTitle("Tox");
    warning.setText(message);
    warning.setIcon(QMessageBox::Warning);
    warning.exec();
}

QString AddFriendForm::getMessage() const
{
    const QString msg = message.toPlainText();
    return !msg.isEmpty() ? msg : message.placeholderText();
}

void AddFriendForm::onSendTriggered()
{
    QString id = toxId.text().trimmed();

    if (id.isEmpty()) {
        showWarning(tr("Please fill in a valid Tox ID","Tox ID of the friend you're sending a friend request to"));
    } else if (isToxId(id)) {
        if (id.toUpper() == Core::getInstance()->getSelfId().toString().toUpper())
            showWarning(tr("You can't add yourself as a friend!","When trying to add your own Tox ID as friend"));
        else
            emit friendRequested(id, getMessage());
        this->toxId.clear();
        this->message.clear();
    } else {
        // If we're querying one of our pinned server, do a tox3 request directly
        QString servname = id.mid(id.indexOf('@')+1);
        for (const AddFriendForm::tox3_server& pin : pinnedServers)
        {
            if (servname == pin.name)
            {
                queryTox3(pin, id);
                return;
            }
        }

        // Otherwise try tox3 if we can get a pubkey or fallback to tox1
        QByteArray pubkey = fetchLastTextRecord("_tox."+servname);
        if (!pubkey.isEmpty())
        {
            QByteArray servnameData = servname.toUtf8();
            AddFriendForm::tox3_server server;
            server.name = servnameData.data();
            server.pubkey = (uint8_t*)pubkey.data();
            queryTox3(server, id);
        }
        else
        {
            queryTox1(id);
        }
    }
}

QByteArray AddFriendForm::fetchLastTextRecord(const QString& record, bool silent)
{
    QByteArray result;

    dns.setName(record);
    dns.lookup();

    int timeout;
    for (timeout = 0; timeout<30 && !dns.isFinished(); ++timeout)
    {
        qApp->processEvents();
        QThread::msleep(100);
    }
    if (timeout >= 30) {
        dns.abort();
        if (!silent)
            showWarning(tr("The connection timed out","The DNS gives the Tox ID associated to toxme.se addresses"));
        return result;
    }

    if (dns.error() == QDnsLookup::NotFoundError) {
        if (!silent)
            showWarning(tr("This address does not exist","The DNS gives the Tox ID associated to toxme.se addresses"));
        return result;
    }
    else if (dns.error() != QDnsLookup::NoError) {
        if (!silent)
            showWarning(tr("Error while looking up DNS","The DNS gives the Tox ID associated to toxme.se addresses"));
        return result;
    }

    const QList<QDnsTextRecord> textRecords = dns.textRecords();
    if (textRecords.isEmpty()) {
        if (!silent)
            showWarning(tr("No text record found", "Error with the DNS"));
        return result;
    }

    const QList<QByteArray> textRecordValues = textRecords.last().values();
    if (textRecordValues.length() != 1) {
        if (!silent)
            showWarning(tr("Unexpected number of values in text record", "Error with the DNS"));
        return result;
    }

    result = textRecordValues.first();
    return result;
}

void AddFriendForm::queryTox1(const QString& record)
{
    QString realRecord = record;
    realRecord.replace("@", "._tox.");
    const QString entry = fetchLastTextRecord(realRecord, false);
    if (entry.isEmpty())
        return;

    // Check toxdns protocol version
    int verx = entry.indexOf("v=");
    if (verx) {
        verx += 2;
        int verend = entry.indexOf(';', verx);
        if (verend)
        {
            QString ver = entry.mid(verx, verend-verx);
            if (ver != "tox1")
            {
                showWarning(tr("The version of Tox DNS used by this server is not supported", "Error with the DNS"));
                return;
            }
        }
    }

    // Get the tox id
    int idx = entry.indexOf("id=");
    if (idx < 0) {
        showWarning(tr("The DNS lookup does not contain any Tox ID", "Error with the DNS"));
        return;
    }

    idx += 3;
    if (entry.length() < idx + static_cast<int>(TOX_ID_LENGTH)) {
        showWarning(tr("The DNS lookup does not contain a valid Tox ID", "Error with the DNS"));
        return;
    }

    const QString friendAdress = entry.mid(idx, TOX_ID_LENGTH);
    if (!isToxId(friendAdress)) {
        showWarning(tr("The DNS lookup does not contain a valid Tox ID", "Error with the DNS"));
        return;
    }

    // finally we got it
    emit friendRequested(friendAdress, getMessage());
    this->toxId.clear();
    this->message.clear();
}

void AddFriendForm::queryTox3(const tox3_server& server, QString &record)
{
    QByteArray nameData = record.left(record.indexOf('@')).toUtf8(), id, realRecord;
    QString entry, toxIdStr;
    int toxIdSize, idx, verx, dns_string_len;
    const int dns_string_maxlen = 128;

    void* tox_dns3 = tox_dns3_new(server.pubkey);
    if (!tox_dns3)
    {
        qWarning() << "queryTox3: failed to create a tox_dns3 object for "<<server.name<<", using tox1 as a fallback";
        goto fallbackOnTox1;
    }
    uint32_t request_id;
    uint8_t dns_string[dns_string_maxlen];
    dns_string_len = tox_generate_dns3_string(tox_dns3, dns_string, dns_string_maxlen, &request_id,
                             (uint8_t*)nameData.data(), nameData.size());

    if (dns_string_len < 0) // We can always fallback on tox1 if toxdns3 fails
    {
        qWarning() << "queryTox3: failed to generate dns3 string for "<<server.name<<", using tox1 as a fallback";
        goto fallbackOnTox1;
    }

    realRecord = '_'+QByteArray((char*)dns_string, dns_string_len)+"._tox."+server.name;
    entry = fetchLastTextRecord(realRecord, false);
    if (entry.isEmpty())
    {
        qWarning() << "queryTox3: Server "<<server.name<<" returned no record, using tox1 as a fallback";
        goto fallbackOnTox1;
    }

    // Check toxdns protocol version
    verx = entry.indexOf("v=");
    if (verx!=-1) {
        verx += 2;
        int verend = entry.indexOf(';', verx);
        if (verend!=-1)
        {
            QString ver = entry.mid(verx, verend-verx);
            if (ver != "tox3")
            {
                qWarning() << "queryTox3: Server "<<server.name<<" returned a bad version ("<<ver<<"), using tox1 as a fallback";
                goto fallbackOnTox1;
            }
        }
    }

    // Get and decrypt the tox id
    idx = entry.indexOf("id=");
    if (idx < 0) {
        qWarning() << "queryTox3: Server "<<server.name<<" returned an empty id, using tox1 as a fallback";
        goto fallbackOnTox1;
    }

    idx += 3;
    id = entry.mid(idx).toUtf8();
    uint8_t toxId[TOX_FRIEND_ADDRESS_SIZE];
    toxIdSize = tox_decrypt_dns3_TXT(tox_dns3, toxId, (uint8_t*)id.data(), id.size(), request_id);
    if (toxIdSize < 0) // We can always fallback on tox1 if toxdns3 fails
    {
        qWarning() << "queryTox3: failed to decrypt dns3 reply for "<<server.name<<", using tox1 as a fallback";
        goto fallbackOnTox1;
    }

    tox_dns3_kill(tox_dns3);
    toxIdStr = CFriendAddress::toString(toxId);
    emit friendRequested(toxIdStr, getMessage());
    this->toxId.clear();
    this->message.clear();
    return;

    // Centralized error handling, fallback on tox1 queries
fallbackOnTox1:
    if (tox_dns3)
        tox_dns3_kill(tox_dns3);
    queryTox1(record);
    return;
}
