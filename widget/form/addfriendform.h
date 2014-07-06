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

#include "ui_widget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QDnsLookup>

class AddFriendForm : public QObject
{
    Q_OBJECT
public:
    AddFriendForm();
    ~AddFriendForm();

    void show(Ui::Widget& ui);
    bool isToxId(const QString& value) const;
    void showWarning(const QString& message) const;
    QString getMessage() const;

signals:
    void friendRequested(const QString& friendAddress, const QString& message);

private slots:
     void onSendTriggered();
     void handleDnsLookup();

private:
    QLabel headLabel, toxIdLabel, messageLabel;
    QPushButton sendButton;
    QLineEdit toxId;
    QTextEdit message;
    QVBoxLayout layout, headLayout;
    QWidget *head, *main;

    /** will be used for dns discovery if necessary */
    QDnsLookup dns;
};

#endif // ADDFRIENDFORM_H
