/*
    Copyright (C) 2013 by Maxim Biro <nurupo.contributions@gmail.com>
    Copyright © 2014-2015 by The qTox Project

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is free software: you can redistribute it and/or modify
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

#include "friendrequestdialog.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>

FriendRequestDialog::FriendRequestDialog(QWidget *parent, const QString &userId, const QString &message) :
    QDialog(parent)
{
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Friend request","Title of the window to aceept/deny a friend request"));

    QLabel *friendsLabel = new QLabel(tr("Someone wants to make friends with you"), this);
    QLabel *userIdLabel = new QLabel(tr("User ID:"), this);
    QLineEdit *userIdEdit = new QLineEdit(userId, this);
    userIdEdit->setCursorPosition(0);
    userIdEdit->setReadOnly(true);
    QLabel *messageLabel = new QLabel(tr("Friend request message:"), this);
    QPlainTextEdit *messageEdit = new QPlainTextEdit(message, this);
    messageEdit->setReadOnly(true);


    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);

    buttonBox->addButton(tr("Accept","Accept a friend request"), QDialogButtonBox::AcceptRole);
    buttonBox->addButton(tr("Reject","Reject a friend request"), QDialogButtonBox::RejectRole);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &FriendRequestDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FriendRequestDialog::reject);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(friendsLabel);
    layout->addSpacing(12);
    layout->addWidget(userIdLabel);
    layout->addWidget(userIdEdit);
    layout->addWidget(messageLabel);
    layout->addWidget(messageEdit);
    layout->addWidget(buttonBox);

    resize(300, 200);
}
