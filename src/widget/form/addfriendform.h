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

#ifndef ADDFRIENDFORM_H
#define ADDFRIENDFORM_H

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QDnsLookup>

namespace Ui {class MainWindow;}

class AddFriendForm : public QObject
{
    Q_OBJECT
public:
    AddFriendForm();
    ~AddFriendForm();

    void show(Ui::MainWindow &ui);
    bool isToxId(const QString& value) const;
    void showWarning(const QString& message) const;
    QString getMessage() const;

signals:
    void friendRequested(const QString& friendAddress, const QString& message);

private slots:
    void onSendTriggered();

private:
    struct tox3_server
    {
        tox3_server()=default;
        tox3_server(const char* _name, uint8_t _pk[32]):name{_name},pubkey{_pk}{}

        const char* name; ///< Hostname of the server, e.g. toxme.se
        uint8_t* pubkey; ///< Public key of the tox3 server, usually 256bit long
    };

private:
    void queryTox1(const QString& record); ///< Record should look like user@domain.tld
    void queryTox3(const tox3_server& server, QString& record); ///< Record should look like user@domain.tld, may fallback on queryTox1
    /// Try to fetch the first entry of the given TXT record
    /// Returns an empty object on failure. May block for up to ~3s
    /// May display message boxes on error if silent if false
    QByteArray fetchLastTextRecord(const QString& record, bool silent=true);

private:
    QLabel headLabel, toxIdLabel, messageLabel;
    QPushButton sendButton;
    QLineEdit toxId;
    QTextEdit message;
    QVBoxLayout layout, headLayout;
    QWidget *head, *main;

    /** will be used for dns discovery if necessary */
    QDnsLookup dns;
    static const tox3_server pinnedServers[];
};

#endif // ADDFRIENDFORM_H
