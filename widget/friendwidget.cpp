#include "friendwidget.h"
#include "group.h"
#include "grouplist.h"
#include "groupwidget.h"
#include "widget.h"
#include <QContextMenuEvent>
#include <QMenu>

FriendWidget::FriendWidget(int FriendId, QString id)
    : friendId(FriendId)
{
    this->setMouseTracking(true);
    this->setAutoFillBackground(true);
    this->setFixedWidth(225);
    this->setFixedHeight(55);
    this->setLayout(&layout);
    layout.setSpacing(0);
    layout.setMargin(0);
    layout.setStretchFactor(this, 100);
    textLayout.setSpacing(0);
    textLayout.setMargin(0);

    avatar.setPixmap(QPixmap(":img/contact.png"));
    name.setText(id);
    //statusPic.setAlignment(Qt::AlignHCenter);
    statusPic.setPixmap(QPixmap(":img/status/dot_away.png"));
    QFont small;
    small.setPixelSize(10);
    statusMessage.setFont(small);
    QPalette pal;
    pal.setColor(QPalette::WindowText,Qt::gray);
    statusMessage.setPalette(pal);
    QPalette pal2;
    pal2.setColor(QPalette::WindowText,Qt::white);
    name.setPalette(pal2);
    QPalette pal3;
    pal3.setColor(QPalette::Background, QColor(65,65,65,255));
    this->setPalette(pal3);

    textLayout.addStretch();
    textLayout.addWidget(&name);
    textLayout.addWidget(&statusMessage);
    textLayout.addStretch();

    layout.addSpacing(20);
    layout.addWidget(&avatar);
    layout.addSpacing(5);
    layout.addLayout(&textLayout);
    layout.addStretch();
    layout.addSpacing(5);
    layout.addWidget(&statusPic);
    layout.addSpacing(5);

    isActiveWidget = 0;
}

void FriendWidget::setNewFixedWidth(int newWidth)
{
    this->setFixedWidth(newWidth);
}

void FriendWidget::mouseReleaseEvent (QMouseEvent*)
{
    emit friendWidgetClicked(this);
}

void FriendWidget::contextMenuEvent(QContextMenuEvent * event)
{
    QPoint pos = event->globalPos();
    QMenu menu;
    menu.addAction("Copy friend ID");
    QMenu* inviteMenu = menu.addMenu("Invite in group");
    QMap<QAction*, Group*> groupActions;
    for (Group* group : GroupList::groupList)
    {
        QAction* groupAction = inviteMenu->addAction(group->widget->name.text());
        groupActions[groupAction] =  group;
    }
    if (groupActions.isEmpty())
        inviteMenu->setEnabled(false);
    menu.addSeparator();
    menu.addAction("Remove friend");

    QAction* selectedItem = menu.exec(pos);
    if (selectedItem)
    {
        if (selectedItem->text() == "Copy friend ID")
        {
            emit copyFriendIdToClipboard(friendId);
            return;
        }
        else if (selectedItem->text() == "Remove friend")
        {
            hide();
            emit removeFriend(friendId);
            return;
        }
        else if (groupActions.contains(selectedItem))
        {
            Group* group = groupActions[selectedItem];
            Widget::getInstance()->getCore()->groupInviteFriend(friendId, group->groupId);
        }
    }
}

void FriendWidget::mousePressEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) == Qt::LeftButton)
    {
        if (isActiveWidget)
        {
            QPalette pal;
            pal.setColor(QPalette::Background, QColor(250,250,250,255));
            this->setPalette(pal);
        }
        else
        {
            QPalette pal;
            pal.setColor(QPalette::Background, QColor(85,85,85,255));
            this->setPalette(pal);
        }
    }
}

void FriendWidget::enterEvent(QEvent*)
{
    if (isActiveWidget != 1)
    {
        QPalette pal;
        pal.setColor(QPalette::Background, QColor(75,75,75,255));
        lastColor = this->palette().background().color();
        this->setPalette(pal);
    }
}

void FriendWidget::leaveEvent(QEvent*)
{
    if (isActiveWidget != 1)
    {
        QPalette pal;
        pal.setColor(QPalette::Background, lastColor);
        this->setPalette(pal);
    }
}


void FriendWidget::setAsActiveChatroom()
{
    isActiveWidget = 1;

    QFont small;
    small.setPixelSize(10);
    statusMessage.setFont(small);
    QPalette pal;
    pal.setColor(QPalette::WindowText,Qt::darkGray);
    statusMessage.setPalette(pal);
    QPalette pal2;
    pal2.setColor(QPalette::WindowText,Qt::black);
    name.setPalette(pal2);
    QPalette pal3;
    pal3.setColor(QPalette::Background, Qt::white);
    this->setPalette(pal3);
    avatar.setPixmap(QPixmap(":img/contact_dark.png"));
}

void FriendWidget::setAsInactiveChatroom()
{
    isActiveWidget = 0;

    QFont small;
    small.setPixelSize(10);
    statusMessage.setFont(small);
    QPalette pal;
    pal.setColor(QPalette::WindowText,Qt::gray);
    statusMessage.setPalette(pal);
    QPalette pal2;
    pal2.setColor(QPalette::WindowText,Qt::white);
    name.setPalette(pal2);
    QPalette pal3;
    pal3.setColor(QPalette::Background, QColor(65,65,65,255));
    this->setPalette(pal3);
    avatar.setPixmap(QPixmap(":img/contact.png"));
}
