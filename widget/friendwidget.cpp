#include "friendwidget.h"
#include <QContextMenuEvent>
#include <QMenu>

FriendWidget::FriendWidget(int FriendId, QString id)
    : friendId(FriendId)
{
    this->setAutoFillBackground(true);
    this->setLayout(&layout);
    this->setFixedWidth(225);
    this->setFixedHeight(55);
    layout.setSpacing(0);
    layout.setMargin(0);
    textLayout.setSpacing(0);
    textLayout.setMargin(0);

    avatar.setPixmap(QPixmap("img/contact list icons/contact.png"));
    name.setText(id);
    statusPic.setPixmap(QPixmap("img/status/dot_away.png"));
    QFont small;
    small.setPixelSize(10);
    statusMessage.setFont(small);
    QPalette pal;
    pal.setColor(QPalette::WindowText,Qt::gray);
    statusMessage.setPalette(pal);

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
}

void FriendWidget::mouseReleaseEvent (QMouseEvent*)
{
    emit friendWidgetClicked(this);
}

void FriendWidget::contextMenuEvent(QContextMenuEvent * event)
{
    QPoint pos = event->globalPos();
    QMenu menu;
    menu.addAction("Remove friend");

    QAction* selectedItem = menu.exec(pos);
    if (selectedItem)
    {
        if (selectedItem->text() == "Remove friend")
        {
            hide();
            emit removeFriend(friendId);
            return;
        }
    }
}

void FriendWidget::setAsActiveFriend()
{
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
}

void FriendWidget::setAsInactiveFriend()
{
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
    pal3.setColor(QPalette::Background, QColor(63,63,63,255));
    this->setPalette(pal3);
}
