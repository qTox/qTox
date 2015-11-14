/*
    Copyright © 2014-2015 by The qTox Project

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
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

#include "addfriendform.h"

#include <QFont>
#include <QMessageBox>
#include <QErrorMessage>
#include <QApplication>
#include <QClipboard>
#include <QRegularExpression>
#include <tox/tox.h>
#include "src/nexus.h"
#include "src/core/core.h"
#include "src/core/cdata.h"
#include "src/net/toxdns.h"
#include "src/persistence/settings.h"
#include "src/widget/gui.h"
#include "src/widget/translator.h"
#include "src/widget/contentlayout.h"
#include <QWindow>

AddFriendForm::AddFriendForm()
{
    main = new QWidget(), head = new QWidget();
    QFont bold;
    bold.setBold(true);
    headLabel.setFont(bold);
    toxIdLabel.setTextFormat(Qt::RichText);

    retranslateUi();

    main->setLayout(&layout);
    layout.addWidget(&toxIdLabel);
    layout.addWidget(&toxId);
    layout.addWidget(&messageLabel);
    layout.addWidget(&message);
    layout.addWidget(&sendButton);

    head->setLayout(&headLayout);
    headLayout.addWidget(&headLabel);

    connect(&toxId, &QLineEdit::returnPressed, this, &AddFriendForm::onSendTriggered);
    connect(&toxId, &QLineEdit::textChanged, this, &AddFriendForm::onIdChanged);
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

bool AddFriendForm::isShown() const
{
    if (main->isVisible())
    {
        head->window()->windowHandle()->alert(0);
        return true;
    }

    return false;
}

void AddFriendForm::show(ContentLayout* contentLayout)
{
    contentLayout->mainContent->layout()->addWidget(main);
    contentLayout->mainHead->layout()->addWidget(head);
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

    if (ToxId::isToxId(id) || (Settings::getInstance().getAllowAddingFriendsPK() && ToxId::isPublicKey(id)))
    {
        if (id.toUpper() == Core::getInstance()->getSelfId().toString().toUpper() || (Settings::getInstance().getAllowAddingFriendsPK() && id.toUpper() == Core::getInstance()->getSelfId().toString().left(2*TOX_PUBLIC_KEY_SIZE).toUpper()))
            GUI::showWarning(tr("Couldn't add friend"), tr("You can't add yourself as a friend!", "When trying to add your own Tox ID as friend"));
        else if (Settings::getInstance().getAllowAddingFriendsPK() && ToxId::isPublicKey(id))
            emit friendNoRequest(id);
        else
        {
            emit friendRequested(id, getMessage());
            this->message.clear();
        }

        this->toxId.clear();
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
            GUI::showWarning(tr("Couldn't add friend"), tr("This Tox ID does not exist", "DNS error"));
            return;
        }

        emit friendRequested(toxId.toString(), getMessage());
        this->toxId.clear();
        this->message.clear();
    }
}

void AddFriendForm::onIdChanged(const QString &id)
{
    QString tId = id.trimmed();
    QRegularExpression dnsIdExpression("^\\S+@\\S+$");
    bool isValidId = tId.isEmpty() || ToxId::isToxId(tId) || tId.contains(dnsIdExpression) || (Settings::getInstance().getAllowAddingFriendsPK() && ToxId::isPublicKey(tId));

    if (Settings::getInstance().getAllowAddingFriendsPK() && ToxId::isPublicKey(tId))
    {
        message.setEnabled(false);
        sendButton.setText(tr("Add friend", "Text when public key is used to add someone"));
    }
    else
    {
        message.setEnabled(true);
        sendButton.setText(tr("Send friend request"));
    }

    QString toxIdText,
        toxIdComment;
    if (Settings::getInstance().getAllowAddingFriendsPK())
    {
        toxIdText = tr("Tox ID or Public Key", "Tox ID or Public Key of the person you're sending a friend request to");
        toxIdComment = tr("either 64, 76 hexadecimal characters or name@example.com", "Public Key and Tox ID format description");
    }
    else
    {
        toxIdText = tr("Tox ID", "Tox ID of the person you're sending a friend request to");
        toxIdComment = tr("either 76 hexadecimal characters or name@example.com", "Tox ID format description");
    }

    if (isValidId)
    {
        toxId.setToolTip(QStringLiteral(""));
        toxIdLabel.setText(toxIdText +
                           QStringLiteral(" (") +
                           toxIdComment +
                           QStringLiteral(")"));
    }
    else
    {
        if (Settings::getInstance().getAllowAddingFriendsPK())
            toxId.setToolTip(tr("Invalid Public Key or Tox ID format"));
        else
            toxId.setToolTip(tr("Invalid Tox ID format"));

        toxIdLabel.setText(toxIdText +
                           QStringLiteral(" <font color='red'>(") +
                           toxIdComment +
                           QStringLiteral(")</font>"));
    }

    toxId.setStyleSheet(isValidId ? QStringLiteral("") : QStringLiteral("QLineEdit { background-color: #FFC1C1; }"));

    sendButton.setEnabled(isValidId && !tId.isEmpty());
}

void AddFriendForm::setIdFromClipboard()
{
    QClipboard* clipboard = QApplication::clipboard();
    QString id = clipboard->text().trimmed();
    if (Core::getInstance()->isReady() && !id.isEmpty() && (ToxId::isToxId(id) || (Settings::getInstance().getAllowAddingFriendsPK() && ToxId::isPublicKey(id))) && ToxId(id) != Core::getInstance()->getSelfId())
        toxId.setText(id);

    onIdChanged(toxId.text());
}

void AddFriendForm::retranslateUi()
{
    headLabel.setText(tr("Add Friends"));
    messageLabel.setText(tr("Message", "The message you send in friend requests"));

    if (Settings::getInstance().getAllowAddingFriendsPK() && ToxId::isPublicKey(toxId.text()))
        sendButton.setText(tr("Add friend", "Text when public key is used to add someone"));
    else
        sendButton.setText(tr("Send friend request"));

    message.setPlaceholderText(tr("%1 here! Tox me maybe?",
                "Default message in friend requests if the field is left blank. Write something appropriate!")
                .arg(lastUsername));

    onIdChanged(toxId.text());
}
