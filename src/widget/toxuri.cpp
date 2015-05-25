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


#include "src/widget/toxuri.h"
#include "src/toxdns.h"
#include "src/widget/tool/friendrequestdialog.h"
#include "src/nexus.h"
#include "src/core/core.h"
#include <QByteArray>
#include <QString>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QCoreApplication>

bool toxURIEventHandler(const QByteArray& eventData)
{
    if (!eventData.startsWith("tox:"))
        return false;

    handleToxURI(eventData);
    return true;
}

bool handleToxURI(const QString &toxURI)
{
    Core* core = Core::getInstance();

    while (!core)
    {
        core = Core::getInstance();
        qApp->processEvents();
    }

    while (!core->isReady())
        qApp->processEvents();

    QString toxaddr;
    if (toxURI.startsWith("tox://"))
        toxaddr = toxURI.mid(6);
    else
        toxaddr = toxURI.mid(4);

    QString toxid = ToxDNS::resolveToxAddress(toxaddr, true).toString();

    if (toxid.isEmpty())
    {
        QMessageBox::warning(0, "qTox", toxaddr+" is not a valid Tox address.");
    }
    else
    {
        ToxURIDialog dialog(0, toxaddr, QObject::tr("%1 here! Tox me maybe?","Default message in Tox URI friend requests. Write something appropriate!").arg(Nexus::getCore()->getUsername()));
        if (dialog.exec() == QDialog::Accepted)
            Core::getInstance()->requestFriendship(toxid, dialog.getRequestMessage());
    }
    return true;
}

ToxURIDialog::ToxURIDialog(QWidget *parent, const QString &userId, const QString &message) :
    QDialog(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Add a friend","Title of the window to add a friend through Tox URI"));

    QLabel *friendsLabel = new QLabel(tr("Do you want to add %1 as a friend?").arg(userId), this);
    QLabel *userIdLabel = new QLabel(tr("User ID:"), this);
    QLineEdit *userIdEdit = new QLineEdit(userId, this);
    userIdEdit->setCursorPosition(0);
    userIdEdit->setReadOnly(true);
    QLabel *messageLabel = new QLabel(tr("Friend request message:"), this);
    messageEdit = new QPlainTextEdit(message, this);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);

    buttonBox->addButton(tr("Send","Send a friend request"), QDialogButtonBox::AcceptRole);
    buttonBox->addButton(tr("Cancel","Don't send a friend request"), QDialogButtonBox::RejectRole);

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

QString ToxURIDialog::getRequestMessage()
{
    return messageEdit->toPlainText();
}
