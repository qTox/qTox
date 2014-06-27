#include "groupwidget.h"
#include "grouplist.h"
#include "group.h"
#include <QPalette>
#include <QMenu>
#include <QContextMenuEvent>

GroupWidget::GroupWidget(int GroupId, QString Name)
    : groupId{GroupId}
{
    this->setAutoFillBackground(true);
    this->setLayout(&layout);
    this->setFixedWidth(225);
    this->setFixedHeight(55);
    layout.setSpacing(0);
    layout.setMargin(0);
    textLayout.setSpacing(0);
    textLayout.setMargin(0);

    avatar.setPixmap(QPixmap("img/contact list icons/group_2x.png"));
    name.setText(Name);
    QFont small;
    small.setPixelSize(10);
    nusers.setFont(small);
    QPalette pal;
    pal.setColor(QPalette::WindowText,Qt::gray);
    nusers.setPalette(pal);
    Group* g = GroupList::findGroup(groupId);
    if (g)
        nusers.setText(QString("%1 users in chat").arg(g->peers.size()));
    else
        nusers.setText("0 users in chat");

    textLayout.addStretch();
    textLayout.addWidget(&name);
    textLayout.addWidget(&nusers);
    textLayout.addStretch();

    layout.addSpacing(20);
    layout.addWidget(&avatar);
    layout.addSpacing(5);
    layout.addLayout(&textLayout);
    layout.addStretch();
}

void GroupWidget::mouseReleaseEvent (QMouseEvent*)
{
    emit groupWidgetClicked(this);
}

void GroupWidget::contextMenuEvent(QContextMenuEvent * event)
{
    QPoint pos = event->globalPos();
    QMenu menu;
    menu.addAction("Quit group");

    QAction* selectedItem = menu.exec(pos);
    if (selectedItem)
    {
        if (selectedItem->text() == "Quit group")
        {
            hide();
            emit removeGroup(groupId);
            return;
        }
    }
}

void GroupWidget::onUserListChanged()
{
    Group* g = GroupList::findGroup(groupId);
    if (g)
        nusers.setText(QString("%1 users in chat").arg(g->nPeers));
    else
        nusers.setText("0 users in chat");
}

void GroupWidget::setAsActiveChatroom()
{
    QFont small;
    small.setPixelSize(10);
    nusers.setFont(small);
    QPalette pal;
    pal.setColor(QPalette::WindowText,Qt::darkGray);
    nusers.setPalette(pal);
    QPalette pal2;
    pal2.setColor(QPalette::WindowText,Qt::black);
    name.setPalette(pal2);
    QPalette pal3;
    pal3.setColor(QPalette::Background, Qt::white);
    this->setPalette(pal3);
}

void GroupWidget::setAsInactiveChatroom()
{
    QFont small;
    small.setPixelSize(10);
    nusers.setFont(small);
    QPalette pal;
    pal.setColor(QPalette::WindowText,Qt::gray);
    nusers.setPalette(pal);
    QPalette pal2;
    pal2.setColor(QPalette::WindowText,Qt::white);
    name.setPalette(pal2);
    QPalette pal3;
    pal3.setColor(QPalette::Background, QColor(63,63,63,255));
    this->setPalette(pal3);
}
