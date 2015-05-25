/*
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

namespace Ui {class MainWindow;}

class AddFriendForm : public QObject
{
    Q_OBJECT
public:
    AddFriendForm();
    ~AddFriendForm();

    void show(Ui::MainWindow &ui);
    QString getMessage() const;

signals:
    void friendRequested(const QString& friendAddress, const QString& message);

public slots:
    void onUsernameSet(const QString& userName);

private slots:
    void onSendTriggered();

private:
    void setIdFromClipboard();
    QLabel headLabel, toxIdLabel, messageLabel;
    QPushButton sendButton;
    QLineEdit toxId;
    QTextEdit message;
    QVBoxLayout layout, headLayout;
    QWidget *head, *main;
};

#endif // ADDFRIENDFORM_H
