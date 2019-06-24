/*
    Copyright © 2014-2019 by The qTox Project Contributors

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
#include "src/core/core.h"
#include "src/net/toxme.h"
#include "src/nexus.h"
#include "src/persistence/settings.h"
#include "src/widget/contentlayout.h"
#include "src/widget/gui.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/widget/style.h"
#include "src/widget/translator.h"
#include <QApplication>
#include <QClipboard>
#include <QErrorMessage>
#include <QFileDialog>
#include <QFont>
#include <QMessageBox>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSignalMapper>
#include <QTabWidget>
#include <QWindow>

namespace
{
    QString getToxId(const QString& id)
    {
        const QString toxUriPrefix{"tox:"};
        QString strippedId = id.trimmed();
        if (strippedId.startsWith(toxUriPrefix)) {
            strippedId.remove(0, toxUriPrefix.length());
        }
        return strippedId;
    }

    bool checkIsValidId(const QString& id)
    {
        static const QRegularExpression dnsIdExpression("^\\S+@\\S+$");
        return ToxId::isToxId(id) || id.contains(dnsIdExpression);
    }
}

/**
 * @var QString AddFriendForm::lastUsername
 * @brief Cached username so we can retranslate the invite message
 */

AddFriendForm::AddFriendForm()
{
    tabWidget = new QTabWidget();
    main = new QWidget(tabWidget), head = new QWidget();
    QFont bold;
    bold.setBold(true);
    headLabel.setFont(bold);
    toxIdLabel.setTextFormat(Qt::RichText);

    main->setLayout(&layout);
    layout.addWidget(&toxIdLabel);
    layout.addWidget(&toxId);
    layout.addWidget(&messageLabel);
    layout.addWidget(&message);
    layout.addWidget(&sendButton);
    tabWidget->addTab(main, QString());

    importContacts = new QWidget(tabWidget);
    importContacts->setLayout(&importContactsLayout);
    importFileLine.addWidget(&importFileLabel);
    importFileLine.addStretch();
    importFileLine.addWidget(&importFileButton);
    importContactsLayout.addLayout(&importFileLine);
    importContactsLayout.addWidget(&importMessageLabel);
    importContactsLayout.addWidget(&importMessage);
    importContactsLayout.addWidget(&importSendButton);
    tabWidget->addTab(importContacts, QString());

    QScrollArea* scrollArea = new QScrollArea(tabWidget);
    QWidget* requestWidget = new QWidget(tabWidget);
    scrollArea->setWidget(requestWidget);
    scrollArea->setWidgetResizable(true);
    requestsLayout = new QVBoxLayout(requestWidget);
    requestsLayout->addStretch(1);
    tabWidget->addTab(scrollArea, QString());

    head->setLayout(&headLayout);
    headLayout.addWidget(&headLabel);

    connect(&toxId, &QLineEdit::returnPressed, this, &AddFriendForm::onSendTriggered);
    connect(&toxId, &QLineEdit::textChanged, this, &AddFriendForm::onIdChanged);
    connect(tabWidget, &QTabWidget::currentChanged, this, &AddFriendForm::onCurrentChanged);
    connect(&sendButton, &QPushButton::clicked, this, &AddFriendForm::onSendTriggered);
    connect(&importSendButton, &QPushButton::clicked, this, &AddFriendForm::onImportSendClicked);
    connect(&importFileButton, &QPushButton::clicked, this, &AddFriendForm::onImportOpenClicked);
    connect(Nexus::getCore(), &Core::usernameSet, this, &AddFriendForm::onUsernameSet);

    // accessibility stuff
    toxIdLabel.setAccessibleDescription(
        tr("Tox ID, either 76 hexadecimal characters or name@example.com"));
    toxId.setAccessibleDescription(tr("Type in Tox ID of your friend"));
    messageLabel.setAccessibleDescription(tr("Friend request message"));
    message.setAccessibleDescription(tr(
        "Type message to send with the friend request or leave empty to send a default message"));
    message.setTabChangesFocus(true);

    retranslateUi();
    Translator::registerHandler(std::bind(&AddFriendForm::retranslateUi, this), this);

    const int size = Settings::getInstance().getFriendRequestSize();
    for (int i = 0; i < size; ++i) {
        Settings::Request request = Settings::getInstance().getFriendRequest(i);
        addFriendRequestWidget(request.address, request.message);
    }
}

AddFriendForm::~AddFriendForm()
{
    Translator::unregister(this);
    head->deleteLater();
    tabWidget->deleteLater();
}

bool AddFriendForm::isShown() const
{
    if (head->isVisible()) {
        head->window()->windowHandle()->alert(0);
        return true;
    }

    return false;
}

void AddFriendForm::show(ContentLayout* contentLayout)
{
    contentLayout->mainContent->layout()->addWidget(tabWidget);
    contentLayout->mainHead->layout()->addWidget(head);
    tabWidget->show();
    head->show();
    setIdFromClipboard();
    toxId.setFocus();

    // Fix #3421
    // Needed to update tab after opening window
    const int index = tabWidget->currentIndex();
    onCurrentChanged(index);
}

QString AddFriendForm::getMessage() const
{
    const QString msg = message.toPlainText();
    return !msg.isEmpty() ? msg : message.placeholderText();
}

QString AddFriendForm::getImportMessage() const
{
    const QString msg = importMessage.toPlainText();
    return msg.isEmpty() ? importMessage.placeholderText() : msg;
}

void AddFriendForm::setMode(Mode mode)
{
    tabWidget->setCurrentIndex(mode);
}

bool AddFriendForm::addFriendRequest(const QString& friendAddress, const QString& message)
{
    if (Settings::getInstance().addFriendRequest(friendAddress, message)) {
        addFriendRequestWidget(friendAddress, message);
        if (isShown()) {
            onCurrentChanged(tabWidget->currentIndex());
        }

        return true;
    }
    return false;
}

void AddFriendForm::onUsernameSet(const QString& username)
{
    lastUsername = username;
    retranslateUi();
}

void AddFriendForm::addFriend(const QString& idText)
{
    ToxId friendId(idText);

    if (!friendId.isValid()) {
        friendId = Toxme::lookup(idText); // Try Toxme
        if (!friendId.isValid()) {
            GUI::showWarning(tr("Couldn't add friend"),
                             tr("%1 Tox ID is invalid or does not exist", "Toxme error").arg(idText));
            return;
        }
    }

    deleteFriendRequest(friendId);
    if (friendId == Core::getInstance()->getSelfId()) {
        GUI::showWarning(tr("Couldn't add friend"),
                         //: When trying to add your own Tox ID as friend
                         tr("You can't add yourself as a friend!"));
    } else {
        emit friendRequested(friendId, getMessage());
    }
}

void AddFriendForm::onSendTriggered()
{
    const QString id = getToxId(toxId.text());
    addFriend(id);

    this->toxId.clear();
    this->message.clear();
}

void AddFriendForm::onImportSendClicked()
{
    for (const QString& id : contactsToImport) {
        addFriend(id);
    }

    contactsToImport.clear();
    importMessage.clear();
    retranslateUi(); // Update the importFileLabel
}

void AddFriendForm::onImportOpenClicked()
{
    const QString path = QFileDialog::getOpenFileName(Q_NULLPTR, tr("Open contact list"));
    if (path.isEmpty()) {
        return;
    }

    QFile contactFile(path);
    if (!contactFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        GUI::showWarning(tr("Couldn't open file"),
                         //: Error message when trying to open a contact list file to import
                         tr("Couldn't open the contact file"));
        return;
    }

    contactsToImport = QString::fromUtf8(contactFile.readAll()).split('\n');
    qDebug() << "Import list:";
    for (auto it = contactsToImport.begin(); it != contactsToImport.end();) {
        const QString id = getToxId(*it);
        if (checkIsValidId(id)) {
            *it = id;
            qDebug() << *it;
            ++it;
        } else {
            if (!id.isEmpty()) {
                qDebug() << "Invalid ID:" << *it;
            }
            it = contactsToImport.erase(it);
        }
    }

    if (contactsToImport.isEmpty()) {
        GUI::showWarning(tr("Invalid file"),
                         tr("We couldn't find any contacts to import in this file!"));
    }

    retranslateUi(); // Update the importFileLabel to show how many contacts we have
}

void AddFriendForm::onIdChanged(const QString& id)
{
    const QString strippedId = getToxId(id);

    const bool isValidId = checkIsValidId(strippedId);
    const bool isValidOrEmpty = strippedId.isEmpty() || isValidId;

    //: Tox ID of the person you're sending a friend request to
    const QString toxIdText(tr("Tox ID"));
    //: Tox ID format description
    const QString toxIdComment(tr("either 76 hexadecimal characters or name@example.com"));

    const QString labelText =
        isValidId ? QStringLiteral("%1 (%2)") : QStringLiteral("%1 <font color='red'>(%2)</font>");
    toxIdLabel.setText(labelText.arg(toxIdText, toxIdComment));
    toxId.setStyleSheet(isValidOrEmpty ? QStringLiteral("")
                                  : Style::getStylesheet("addFriendForm/toxId.css"));
    toxId.setToolTip(isValidOrEmpty ? QStringLiteral("") : tr("Invalid Tox ID format"));

    sendButton.setEnabled(isValidId);
}

void AddFriendForm::setIdFromClipboard()
{
    const QClipboard* clipboard = QApplication::clipboard();
    const QString trimmedId = clipboard->text().trimmed();
    const QString strippedId = getToxId(trimmedId);
    const Core* core = Core::getInstance();
    const bool isSelf = ToxId::isToxId(strippedId) && ToxId(strippedId) != core->getSelfId();
    if (!strippedId.isEmpty() && ToxId::isToxId(strippedId) && isSelf) {
        toxId.setText(trimmedId);
    }
}

void AddFriendForm::deleteFriendRequest(const ToxId& toxId)
{
    const int size = Settings::getInstance().getFriendRequestSize();
    for (int i = 0; i < size; ++i) {
        Settings::Request request = Settings::getInstance().getFriendRequest(i);
        if (toxId == ToxId(request.address)) {
            Settings::getInstance().removeFriendRequest(i);
            return;
        }
    }
}

void AddFriendForm::onFriendRequestAccepted()
{
    QPushButton* acceptButton = static_cast<QPushButton*>(sender());
    QWidget* friendWidget = acceptButton->parentWidget();
    const int index = requestsLayout->indexOf(friendWidget);
    removeFriendRequestWidget(friendWidget);
    const int indexFromEnd = requestsLayout->count() - index - 1;
    const Settings::Request request = Settings::getInstance().getFriendRequest(indexFromEnd);
    emit friendRequestAccepted(ToxId(request.address).getPublicKey());
    Settings::getInstance().removeFriendRequest(indexFromEnd);
    Settings::getInstance().savePersonal();
}

void AddFriendForm::onFriendRequestRejected()
{
    QPushButton* rejectButton = static_cast<QPushButton*>(sender());
    QWidget* friendWidget = rejectButton->parentWidget();
    const int index = requestsLayout->indexOf(friendWidget);
    removeFriendRequestWidget(friendWidget);
    const int indexFromEnd = requestsLayout->count() - index - 1;
    Settings::getInstance().removeFriendRequest(indexFromEnd);
    Settings::getInstance().savePersonal();
}

void AddFriendForm::onCurrentChanged(int index)
{
    if (index == FriendRequest && Settings::getInstance().getUnreadFriendRequests() != 0) {
        Settings::getInstance().clearUnreadFriendRequests();
        Settings::getInstance().savePersonal();
        emit friendRequestsSeen();
    }
}

void AddFriendForm::retranslateUi()
{
    headLabel.setText(tr("Add Friends"));
    //: The message you send in friend requests
    static const QString messageLabelText = tr("Message");
    messageLabel.setText(messageLabelText);
    importMessageLabel.setText(messageLabelText);
    //: Button to choose a file with a list of contacts to import
    importFileButton.setText(tr("Open"));
    importSendButton.setText(tr("Send friend requests"));
    sendButton.setText(tr("Send friend request"));
    //: Default message in friend requests if the field is left blank. Write something appropriate!
    message.setPlaceholderText(tr("%1 here! Tox me maybe?").arg(lastUsername));
    importMessage.setPlaceholderText(message.placeholderText());

    importFileLabel.setText(
        contactsToImport.isEmpty()
            ? tr("Import a list of contacts, one Tox ID per line")
            //: Shows the number of contacts we're about to import from a file (at least one)
            : tr("Ready to import %n contact(s), click send to confirm", "", contactsToImport.size()));

    onIdChanged(toxId.text());

    tabWidget->setTabText(AddFriend, tr("Add a friend"));
    tabWidget->setTabText(ImportContacts, tr("Import contacts"));
    tabWidget->setTabText(FriendRequest, tr("Friend requests"));

    for (QPushButton* acceptButton : acceptButtons) {
        retranslateAcceptButton(acceptButton);
    }

    for (QPushButton* rejectButton : rejectButtons) {
        retranslateRejectButton(rejectButton);
    }
}

void AddFriendForm::addFriendRequestWidget(const QString& friendAddress, const QString& message)
{
    QWidget* friendWidget = new QWidget(tabWidget);
    QHBoxLayout* friendLayout = new QHBoxLayout(friendWidget);
    QVBoxLayout* horLayout = new QVBoxLayout();
    horLayout->setMargin(0);
    friendLayout->addLayout(horLayout);

    CroppingLabel* friendLabel = new CroppingLabel(friendWidget);
    friendLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    friendLabel->setText("<b>" + friendAddress + "</b>");
    horLayout->addWidget(friendLabel);

    QLabel* messageLabel = new QLabel(message);
    // allow to select text, but treat links as plaintext to prevent phishing
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    messageLabel->setTextFormat(Qt::PlainText);
    messageLabel->setWordWrap(true);
    horLayout->addWidget(messageLabel, 1);

    QPushButton* acceptButton = new QPushButton(friendWidget);
    acceptButtons.append(acceptButton);
    connect(acceptButton, &QPushButton::released, this, &AddFriendForm::onFriendRequestAccepted);
    friendLayout->addWidget(acceptButton);
    retranslateAcceptButton(acceptButton);

    QPushButton* rejectButton = new QPushButton(friendWidget);
    rejectButtons.append(rejectButton);
    connect(rejectButton, &QPushButton::released, this, &AddFriendForm::onFriendRequestRejected);
    friendLayout->addWidget(rejectButton);
    retranslateRejectButton(rejectButton);

    requestsLayout->insertWidget(0, friendWidget);
}

void AddFriendForm::removeFriendRequestWidget(QWidget* friendWidget)
{
    int index = requestsLayout->indexOf(friendWidget);
    requestsLayout->removeWidget(friendWidget);
    acceptButtons.removeAt(index);
    rejectButtons.removeAt(index);
    friendWidget->deleteLater();
}

void AddFriendForm::retranslateAcceptButton(QPushButton* acceptButton)
{
    acceptButton->setText(tr("Accept"));
}

void AddFriendForm::retranslateRejectButton(QPushButton* rejectButton)
{
    rejectButton->setText(tr("Reject"));
}
