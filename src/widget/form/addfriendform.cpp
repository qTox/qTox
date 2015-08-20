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
#include <QClipboard>
#include <QTabWidget>
#include <QSignalMapper>
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
    tabWidget = new QTabWidget();
    main = new QWidget(tabWidget), head = new QWidget();
    QFont bold;
    bold.setBold(true);
    headLabel.setFont(bold);

    retranslateUi();

    tabWidget->addTab(main, tr("Add a friend"));
    QScrollArea* scrollArea = new QScrollArea(tabWidget);
    QWidget* requestWidget = new QWidget(tabWidget);
    scrollArea->setWidget(requestWidget);
    scrollArea->setWidgetResizable(true);
    requestsLayout = new QVBoxLayout(requestWidget);
    requestsLayout->addStretch(1);
    tabWidget->addTab(scrollArea, tr("Friend requests"));

    main->setLayout(&layout);
    layout.addWidget(&toxIdLabel);
    layout.addWidget(&toxId);
    layout.addWidget(&messageLabel);
    layout.addWidget(&message);
    layout.addWidget(&sendButton);

    head->setLayout(&headLayout);
    headLayout.addWidget(&headLabel);

    connect(tabWidget, &QTabWidget::currentChanged, this, &AddFriendForm::onCurrentChanged);
    connect(&toxId,&QLineEdit::returnPressed, this, &AddFriendForm::onSendTriggered);
    connect(&sendButton, SIGNAL(clicked()), this, SLOT(onSendTriggered()));
    connect(Nexus::getCore(), &Core::usernameSet, this, &AddFriendForm::onUsernameSet);

    Translator::registerHandler(std::bind(&AddFriendForm::retranslateUi, this), this);

    acceptMapper = new QSignalMapper(requestWidget);
    rejectMapper = new QSignalMapper(requestWidget);
    connect(acceptMapper, SIGNAL(mapped(QWidget*)), this, SLOT(onFriendRequestAccepted(QWidget*)));
    connect(rejectMapper, SIGNAL(mapped(QWidget*)), this, SLOT(onFriendRequestRejected(QWidget*)));
    int size = Settings::getInstance().getFriendRequestSize();

    for (int i = 0; i < size; ++i)
    {
        QPair<QString, QString> request = Settings::getInstance().getFriendRequest(i);
        addFriendRequestWidget(request.first, request.second);
    }
}

AddFriendForm::~AddFriendForm()
{
    Translator::unregister(this);
    head->deleteLater();
    tabWidget->deleteLater();
}

void AddFriendForm::show(Ui::MainWindow &ui)
{
    ui.mainContent->layout()->addWidget(tabWidget);
    ui.mainHead->layout()->addWidget(head);
    tabWidget->show();
    head->show();
    setIdFromClipboard();
    toxId.setFocus();
}

QString AddFriendForm::getMessage() const
{
    const QString msg = message.toPlainText();
    return !msg.isEmpty() ? msg : message.placeholderText();
}

void AddFriendForm::setMode(Mode mode)
{
    tabWidget->setCurrentIndex(mode);
}

void AddFriendForm::addFriendRequest(const QString &friendAddress, const QString &message)
{
    addFriendRequestWidget(friendAddress, message);
    Settings::getInstance().addFriendRequest(friendAddress, message);
    onCurrentChanged(tabWidget->currentIndex());
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
#include <QDebug>
void AddFriendForm::onFriendRequestAccepted(QWidget* friendWidget)
{
    int index = requestsLayout->indexOf(friendWidget);
    friendWidget->deleteLater();
    requestsLayout->removeWidget(friendWidget);
    emit friendRequestAccepted(Settings::getInstance().getFriendRequest(requestsLayout->count() - index - 1).first);
    qDebug() << "Accepted:" << requestsLayout->count() - index - 1;
}

void AddFriendForm::onFriendRequestRejected(QWidget* friendWidget)
{
    int index = requestsLayout->indexOf(friendWidget);
    friendWidget->deleteLater();
    requestsLayout->removeWidget(friendWidget);
    Settings::getInstance().removeFriendRequest(requestsLayout->count() - index - 1);
    Settings::getInstance().savePersonal();
    qDebug() << "Rejected:" << requestsLayout->count() - index - 1;
}

void AddFriendForm::onCurrentChanged(int index)
{
    if (index == FriendRequest && Settings::getInstance().getUnreadFriendRequests() != 0)
    {
        Settings::getInstance().clearUnreadFriendRequests();
        Settings::getInstance().savePersonal();
        emit friendRequestsSeen();
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

void AddFriendForm::addFriendRequestWidget(const QString &friendAddress, const QString &message)
{
    QWidget* friendWidget = new QWidget(tabWidget);
    QHBoxLayout* friendLayout = new QHBoxLayout(friendWidget);
    QVBoxLayout* horLayout = new QVBoxLayout();
    horLayout->setMargin(0);
    friendLayout->addLayout(horLayout);
    CroppingLabel* friendLabel = new CroppingLabel(friendWidget);
    friendLabel->setText("<b>" "12345678901234567890" "</b>");
    horLayout->addWidget(friendLabel);
    QLabel* messageLabel = new QLabel(message);
    messageLabel->setWordWrap(true);
    horLayout->addWidget(messageLabel, 1);
    QPushButton* acceptButton = new QPushButton(tr("Accept"));
    connect(acceptButton, SIGNAL(pressed()), acceptMapper,SLOT(map()));
    acceptMapper->setMapping(acceptButton, friendWidget);
    friendLayout->addWidget(acceptButton);
    QPushButton* rejectButton = new QPushButton(tr("Reject"));
    connect(rejectButton, SIGNAL(pressed()), rejectMapper,SLOT(map()));
    rejectMapper->setMapping(rejectButton, friendWidget);
    friendLayout->addWidget(rejectButton);
    requestsLayout->insertWidget(0, friendWidget);
}
