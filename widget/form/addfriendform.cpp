#include "addfriendform.h"

#include <QFont>
#include <QMessageBox>

#define TOX_ID_SIZE 76

AddFriendForm::AddFriendForm() : dns(this)
{
    dns.setType(QDnsLookup::TXT);

    main = new QWidget(), head = new QWidget();
    QFont bold;
    bold.setBold(true);
    headLabel.setText("Add Friends");
    headLabel.setFont(bold);

    toxIdLabel.setText("Tox ID");
    messageLabel.setText("Message");
    sendButton.setText("Send friend request");

    main->setLayout(&layout);
    layout.addWidget(&toxIdLabel);
    layout.addWidget(&toxId);
    layout.addWidget(&messageLabel);
    layout.addWidget(&message);
    layout.addWidget(&sendButton);

    head->setLayout(&headLayout);
    headLayout.addWidget(&headLabel);

    connect(&sendButton, SIGNAL(clicked()), this, SLOT(onSendTriggered()));
    connect(&dns, SIGNAL(finished()), this, SLOT(handleDnsLookup()));
}

AddFriendForm::~AddFriendForm()
{
    head->deleteLater();
    main->deleteLater();
}

void AddFriendForm::show(Ui::Widget &ui)
{
    ui.mainContent->layout()->addWidget(main);
    ui.mainHead->layout()->addWidget(head);
    main->show();
    head->show();
}

bool AddFriendForm::isToxId(const QString &value) const
{
    const QRegularExpression hexRegExp("^[A-Fa-f0-9]+$");
    return value.length() == TOX_ID_SIZE && value.contains(hexRegExp);
}

void AddFriendForm::showWarning(const QString &message) const
{
    QMessageBox warning(main);
    warning.setText(message);
    warning.setIcon(QMessageBox::Warning);
    warning.exec();
}

QString AddFriendForm::getMessage() const
{
    const QString msg = message.toPlainText();
    return !msg.isEmpty() ? msg : "Tox me maybe?";
}

void AddFriendForm::onSendTriggered()
{
    QString id = toxId.text().trimmed();

    if (id.isEmpty()) {
        showWarning("Please fill in a valid Tox ID");
    } else if (isToxId(id)) {
        emit friendRequested(id, getMessage());
    } else {
        id = id.replace("@", "._tox.");
        dns.setName(id);
        dns.lookup();
    }
}

void AddFriendForm::handleDnsLookup()
{
    const QString idKeyWord("id=");

    if (dns.error() != QDnsLookup::NoError) {
        showWarning("Error while looking up DNS");
        return;
    }

    const QList<QDnsTextRecord> textRecords = dns.textRecords();
    if (textRecords.length() != 1) {
        showWarning("Unexpected number of text records");
        return;
    }

    const QList<QByteArray> textRecordValues = textRecords.first().values();
    if (textRecordValues.length() != 1) {
        showWarning("Unexpected number of values in text record");
        return;
    }

    const QString entry(textRecordValues.first());
    int idx = entry.indexOf(idKeyWord);
    if (idx < 0) {
        showWarning("The DNS lookup does not contain any Tox ID");
        return;
    }

    idx += idKeyWord.length();
    if (entry.length() < idx + static_cast<int>(TOX_ID_SIZE)) {
        showWarning("The DNS lookup does not contain a valid Tox ID");
        return;
    }

    const QString friendAdress = entry.mid(idx, TOX_ID_SIZE);
    if (!isToxId(friendAdress)) {
        showWarning("The DNS lookup does not contain a valid Tox ID");
        return;
    }

    // finally we got it
    emit friendRequested(friendAdress, getMessage());
}
