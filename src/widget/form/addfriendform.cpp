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

#include "addfriendform.h"

#include <QFont>
#include <QMessageBox>
#include <QErrorMessage>
#include <QClipboard>
#include <tox/tox.h>
#include "ui_mainwindow.h"
#include "src/nexus.h"
#include "src/core/core.h"
#include "src/core/cdata.h"
#include "src/net/toxdns.h"
#include "src/persistence/settings.h"
#include "src/widget/gui.h"
#include "src/widget/translator.h"

AddFriendForm::AddFriendForm()
{
    main = new QWidget(), head = new QWidget();
    QFont bold;
    bold.setBold(true);
    headLabel.setFont(bold);

    retranslateUi();

    main->setLayout(&layout);
    layout.addWidget(&toxIdLabel);
    layout.addWidget(&toxId);
    layout.addWidget(&messageLabel);
    layout.addWidget(&message);
    layout.addWidget(&sendButton);

    head->setLayout(&headLayout);
    headLayout.addWidget(&headLabel);

    connect(&toxId,&QLineEdit::returnPressed, this, &AddFriendForm::onSendTriggered);
    connect(&sendButton, SIGNAL(clicked()), this, SLOT(onSendTriggered()));
    connect(Nexus::getCore(), &Core::usernameSet, this, &AddFriendForm::onUsernameSet);

    Translator::registerHandler(std::bind(&AddFriendForm::retranslateUi, this), this);
}

AddFriendForm::~AddFriendForm()
{
    Translator::unregister(this);
    head->deleteLater();
    main->deleteLater();
}

void AddFriendForm::show(Ui::MainWindow &ui)
{
    ui.mainContent->layout()->addWidget(main);
    ui.mainHead->layout()->addWidget(head);
    main->show();
    head->show();
    setIdFromClipboard();
    toxId.setFocus();
}

QString AddFriendForm::getMessage() const
{
    const QString msg = message.toPlainText();
    return !msg.isEmpty() ? msg : message.placeholderText();
}

void AddFriendForm::onUsernameSet(const QString& username)
{
    lastUsername = username;
    retranslateUi();
}

void AddFriendForm::onSendTriggered()
{
    QString id = toxId.text().trimmed();

    if (id.isEmpty())
    {
        GUI::showWarning(tr("Couldn't add friend"), tr("Please fill in a valid Tox ID","Tox ID of the friend you're sending a friend request to"));
    }
    else if (ToxId::isToxId(id))
    {
        if (id.toUpper() == Core::getInstance()->getSelfId().toString().toUpper())
            GUI::showWarning(tr("Couldn't add friend"), tr("You can't add yourself as a friend!","When trying to add your own Tox ID as friend"));
        else
            emit friendRequested(id, getMessage());

        this->toxId.clear();
        this->message.clear();
    }
    else
    {
        if (Settings::getInstance().getProxyType() != ProxyType::ptNone)
        {
            QMessageBox::StandardButton btn = QMessageBox::warning(main, "qTox", tr("qTox needs to use the Tox DNS, but can't do it through a proxy.\n\
Ignore the proxy and connect to the Internet directly?"), QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
            if (btn != QMessageBox::Yes)
                return;
        }

        ToxId toxId = ToxDNS::resolveToxAddress(id, true);

        if (toxId.toString().isEmpty())
        {
            GUI::showWarning(tr("Couldn't add friend"), tr("This Tox ID does not exist","DNS error"));
            return;
        }

        emit friendRequested(toxId.toString(), getMessage());
        this->toxId.clear();
        this->message.clear();
    }
}

void AddFriendForm::setIdFromClipboard()
{
    QClipboard* clipboard = QApplication::clipboard();
    QString id = clipboard->text().trimmed();
    if (Core::getInstance()->isReady() && !id.isEmpty() && ToxId::isToxId(id))
    {
        if (!ToxId(id).isActiveProfile())
            toxId.setText(id);
    }
}

void AddFriendForm::retranslateUi()
{
    headLabel.setText(tr("Add Friends"));
    toxIdLabel.setText(tr("Tox ID","Tox ID of the person you're sending a friend request to"));
    messageLabel.setText(tr("Message","The message you send in friend requests"));
    sendButton.setText(tr("Send friend request"));
    message.setPlaceholderText(tr("%1 here! Tox me maybe?",
                "Default message in friend requests if the field is left blank. Write something appropriate!")
                .arg(lastUsername));
}
