#include "groupchatform.h"
#include "group.h"
#include "groupwidget.h"
#include "widget.h"
#include "friend.h"
#include "friendlist.h"
#include <QFont>
#include <QTime>
#include <QScrollBar>

GroupChatForm::GroupChatForm(Group* chatGroup)
    : group(chatGroup), curRow{0}, lockSliderToBottom{true}
{
    main = new QWidget(), head = new QWidget(), chatAreaWidget = new QWidget();
    headLayout = new QHBoxLayout(), mainFootLayout = new QHBoxLayout();
    headTextLayout = new QVBoxLayout(), mainLayout = new QVBoxLayout();
    mainChatLayout = new QGridLayout();
    avatar = new QLabel(), name = new QLabel(), nusers = new QLabel(), namesList = new QLabel();
    msgEdit = new ChatTextEdit();
    sendButton = new QPushButton();
    chatArea = new QScrollArea();
    QFont bold;
    bold.setBold(true);
    QFont small;
    small.setPixelSize(10);
    name->setText(group->widget->name.text());
    name->setFont(bold);
    nusers->setFont(small);
    nusers->setText(QString("%1 users in chat").arg(group->peers.size()));
    avatar->setPixmap(QPixmap("img/contact list icons/group.png"));
    QString names;
    for (QString& s : group->peers)
        names.append(s+", ");
    names.chop(2);
    namesList->setText(names);
    namesList->setFont(small);

    chatAreaWidget->setLayout(mainChatLayout);
    chatArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    chatArea->setWidgetResizable(true);
    mainChatLayout->setColumnStretch(1,1);
    mainChatLayout->setHorizontalSpacing(10);

    sendButton->setIcon(QIcon("img/button icons/sendmessage_2x.png"));
    sendButton->setFlat(true);
    QPalette pal;
    pal.setColor(QPalette::Button, QColor(107,194,96)); // Tox Green
    sendButton->setPalette(pal);
    sendButton->setAutoFillBackground(true);
    msgEdit->setFixedHeight(50);
    sendButton->setFixedSize(50, 50);

    main->setLayout(mainLayout);
    mainLayout->addWidget(chatArea);
    mainLayout->addLayout(mainFootLayout);
    mainLayout->setMargin(0);

    mainFootLayout->addWidget(msgEdit);
    mainFootLayout->addWidget(sendButton);

    head->setLayout(headLayout);
    headLayout->addWidget(avatar);
    headLayout->addLayout(headTextLayout);
    headLayout->addStretch();
    headLayout->setMargin(0);

    headTextLayout->addStretch();
    headTextLayout->addWidget(name);
    headTextLayout->addWidget(nusers);
    headTextLayout->addWidget(namesList);
    headTextLayout->setMargin(0);
    headTextLayout->setSpacing(0);
    headTextLayout->addStretch();

    chatArea->setWidget(chatAreaWidget);

    connect(sendButton, SIGNAL(clicked()), this, SLOT(onSendTriggered()));
    connect(msgEdit, SIGNAL(enterPressed()), this, SLOT(onSendTriggered()));
    connect(chatArea->verticalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(onSliderRangeChanged()));
}

GroupChatForm::~GroupChatForm()
{
    delete head;
    delete main;
}

void GroupChatForm::show(Ui::Widget &ui)
{
    ui.mainContent->layout()->addWidget(main);
    ui.mainHead->layout()->addWidget(head);
    main->show();
    head->show();
}

void GroupChatForm::setName(QString newName)
{
    name->setText(newName);
}

void GroupChatForm::onSendTriggered()
{
    QString msg = msgEdit->toPlainText();
    if (msg.isEmpty())
        return;
    msgEdit->clear();
    emit sendMessage(group->groupId, msg);
}

void GroupChatForm::addGroupMessage(QString message, int peerId)
{
    QLabel *msgAuthor;
    if (group->peers.contains(peerId))
        msgAuthor = new QLabel(group->peers[peerId]);
    else
        msgAuthor = new QLabel("<Unknown>");

    QLabel *msgText = new QLabel(message);
    QLabel *msgDate = new QLabel(QTime::currentTime().toString("hh:mm"));

    addMessage(msgAuthor, msgText, msgDate);
}

void GroupChatForm::addMessage(QString author, QString message, QString date)
{
    addMessage(new QLabel(author), new QLabel(message), new QLabel(date));
}

void GroupChatForm::addMessage(QLabel* author, QLabel* message, QLabel* date)
{
    QScrollBar* scroll = chatArea->verticalScrollBar();
    lockSliderToBottom = scroll && scroll->value() == scroll->maximum();
    author->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    date->setAlignment(Qt::AlignTop);
    if (author->text() == Widget::getInstance()->getUsername())
    {
        QPalette pal;
        pal.setColor(QPalette::WindowText, Qt::gray);
        author->setPalette(pal);
        message->setPalette(pal);
    }
    if (previousName.isEmpty() || previousName != author->text())
    {
        if (curRow)
        {
            mainChatLayout->setRowStretch(curRow, 0);
            mainChatLayout->addItem(new QSpacerItem(0,AUTHOR_CHANGE_SPACING),curRow,0,1,3);
            curRow++;
        }
        mainChatLayout->addWidget(author, curRow, 0);
    }
    previousName = author->text();
    mainChatLayout->addWidget(message, curRow, 1);
    mainChatLayout->addWidget(date, curRow, 3);
    mainChatLayout->setRowStretch(curRow+1, 1);
    mainChatLayout->setRowStretch(curRow, 0);
    curRow++;
}

void GroupChatForm::onSliderRangeChanged()
{
    QScrollBar* scroll = chatArea->verticalScrollBar();
    if (lockSliderToBottom)
         scroll->setValue(scroll->maximum());
}

void GroupChatForm::onUserListChanged()
{
    nusers->setText(QString("%1 users in chat").arg(group->nPeers));
    QString names;
    for (QString& s : group->peers)
        names.append(s+", ");
    names.chop(2);
    namesList->setText(names);
}
