#include "settingsform.h"
#include <QFont>

SettingsForm::SettingsForm()
    : QObject()
{
    main = new QWidget(), head = new QWidget();
    QFont bold, small;
    bold.setBold(true);
    small.setPixelSize(7);
    headLabel.setText("User Settings");
    headLabel.setFont(bold);

    nameLabel.setText("Name");
    statusTextLabel.setText("Status");
    idLabel.setText("Tox ID");
    id.setFont(small);
    id.setTextInteractionFlags(Qt::TextSelectableByMouse);

    main->setLayout(&layout);
    layout.addWidget(&nameLabel);
    layout.addWidget(&name);
    layout.addWidget(&statusTextLabel);
    layout.addWidget(&statusText);
    layout.addWidget(&idLabel);
    layout.addWidget(&id);
    layout.addStretch();

    head->setLayout(&headLayout);
    headLayout.addWidget(&headLabel);
}

SettingsForm::~SettingsForm()
{
}

void SettingsForm::setFriendAddress(const QString& friendAddress)
{
    id.setText(friendAddress);
}

void SettingsForm::show(Ui::Widget &ui)
{
    name.setText(ui.nameLabel->text());
    statusText.setText(ui.statusLabel->text());
    ui.mainContent->layout()->addWidget(main);
    ui.mainHead->layout()->addWidget(head);
    main->show();
    head->show();
}
