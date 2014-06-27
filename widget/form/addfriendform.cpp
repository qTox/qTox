#include "addfriendform.h"
#include "ui_widget.h"
#include <QFont>

AddFriendForm::AddFriendForm()
{
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
}

void AddFriendForm::show(Ui::Widget &ui)
{
    ui.mainContent->layout()->addWidget(main);
    ui.mainHead->layout()->addWidget(head);
    main->show();
    head->show();
}

void AddFriendForm::onSendTriggered()
{
    QString id = toxId.text(), msg = message.toPlainText();
    if (id.isEmpty())
        return;
    if (msg.isEmpty())
        msg = "Tox me maybe?";

    emit friendRequested(id, msg);
}
