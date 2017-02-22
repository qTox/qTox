/*
    Copyright © 2015 by The qTox Project Contributors

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

#include "groupinviteform.h"

#include "src/widget/form/groupinvitewidget.h"
#include "src/core/core.h"
#include "src/nexus.h"
#include "src/persistence/settings.h"
#include "src/widget/contentlayout.h"
#include "src/widget/translator.h"
#include "ui_mainwindow.h"

#include <algorithm>
#include <tox/tox.h>

#include <QVBoxLayout>
#include <QDateTime>
#include <QDebug>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSignalMapper>
#include <QWindow>

GroupInviteForm::GroupInviteForm()
    : createButton(new QPushButton(this))
    , inviteBox(new QGroupBox(this))
    , scroll(new QScrollArea(this))
    , headLabel(new QLabel(this))
    , headWidget(new QWidget(this))
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    connect(createButton, &QPushButton::clicked, [=]()
    {
        emit groupCreate(TOX_CONFERENCE_TYPE_AV);
    });

    QWidget* innerWidget = new QWidget(scroll);
    innerWidget->setLayout(new QVBoxLayout());
    innerWidget->layout()->setAlignment(Qt::AlignTop);
    scroll->setWidget(innerWidget);
    scroll->setWidgetResizable(true);

    QVBoxLayout* inviteLayout = new QVBoxLayout(inviteBox);
    inviteLayout->addWidget(scroll);

    layout->addWidget(createButton);
    layout->addWidget(inviteBox);

    QFont bold;
    bold.setBold(true);

    headLabel->setFont(bold);
    QHBoxLayout* headLayout = new QHBoxLayout(headWidget);
    headLayout->addWidget(headLabel);

    retranslateUi();
    Translator::registerHandler(std::bind(&GroupInviteForm::retranslateUi, this), this);
}

GroupInviteForm::~GroupInviteForm()
{
    Translator::unregister(this);
}

bool GroupInviteForm::isShown() const
{
    bool result = isVisible();
    if (result)
    {
        headWidget->window()->windowHandle()->alert(0);
    }
    return result;
}

void GroupInviteForm::show(ContentLayout* contentLayout)
{
    contentLayout->mainContent->layout()->addWidget(this);
    contentLayout->mainHead->layout()->addWidget(headWidget);
    QWidget::show();
    headWidget->show();
}

void GroupInviteForm::addGroupInvite(int32_t friendId, uint8_t type, QByteArray invite)
{
    GroupInviteWidget* addingInviteWidget = new GroupInviteWidget(this, GroupInvite(friendId, type, invite));
    scroll->widget()->layout()->addWidget(addingInviteWidget);
    invites.append(addingInviteWidget);
    connect(addingInviteWidget, &GroupInviteWidget::accepted,
            [=] (const GroupInvite& inviteInfo) {
        deleteInviteWidget(inviteInfo);
        emit groupInviteAccepted(inviteInfo.getFriendId(),
                                 inviteInfo.getType(),
                                 inviteInfo.getInvite());
    });
    connect(addingInviteWidget, &GroupInviteWidget::rejected,
            [=] (const GroupInvite& inviteInfo) {
        deleteInviteWidget(inviteInfo);
    });
    if (isVisible()) {
        emit groupInvitesSeen();
    }
}

void GroupInviteForm::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    emit groupInvitesSeen();
}

void GroupInviteForm::deleteInviteWidget(const GroupInvite &inviteInfo)
{
    auto deletingWidget = std::find_if(invites.begin(), invites.end(),
                                       [=] (const GroupInviteWidget* widget) {
        return inviteInfo == widget->getInviteInfo();
    });
    (*deletingWidget)->deleteLater();
    scroll->widget()->layout()->removeWidget(*deletingWidget);
    invites.erase(deletingWidget);
}

void GroupInviteForm::retranslateUi()
{
    headLabel->setText(tr("Groups"));
    if (createButton)
    {
        createButton->setText(tr("Create new group"));
    }
    inviteBox->setTitle(tr("Group invites"));
    for (GroupInviteWidget* invite : invites)
    {
        invite->retranslateUi();
    }
}
