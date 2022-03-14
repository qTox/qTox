/*
    Copyright © 2015-2019 by The qTox Project Contributors

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

#include "ui_mainwindow.h"
#include "src/core/core.h"
#include "src/model/groupinvite.h"
#include "src/persistence/settings.h"
#include "src/widget/contentlayout.h"
#include "src/widget/form/groupinvitewidget.h"
#include "src/widget/translator.h"

#include <QDateTime>
#include <QDebug>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSignalMapper>
#include <QVBoxLayout>
#include <QWindow>

#include <algorithm>
#include <tox/tox.h>

/**
 * @class GroupInviteForm
 *
 * @brief This form contains all group invites you received
 */

GroupInviteForm::GroupInviteForm(Settings& settings_, Core& core_)
    : headWidget(new QWidget(this))
    , headLabel(new QLabel(this))
    , createButton(new QPushButton(this))
    , inviteBox(new QGroupBox(this))
    , scroll(new QScrollArea(this))
    , settings{settings_}
    , core{core_}
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    connect(createButton, &QPushButton::clicked,
            [this]() { emit groupCreate(TOX_CONFERENCE_TYPE_AV); });

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

/**
 * @brief Detects that form is shown
 * @return True if form is visible
 */
bool GroupInviteForm::isShown() const
{
    bool result = isVisible();
    if (result) {
        headWidget->window()->windowHandle()->alert(0);
    }
    return result;
}

/**
 * @brief Shows the form
 * @param contentLayout Main layout that contains all components of the form
 */
void GroupInviteForm::show(ContentLayout* contentLayout)
{
    contentLayout->mainContent->layout()->addWidget(this);
    contentLayout->mainHead->layout()->addWidget(headWidget);
    QWidget::show();
    headWidget->show();
}

/**
 * @brief Adds group invite
 * @param inviteInfo Object which contains info about group invitation
 * @return true if notification is needed, false otherwise
 */
bool GroupInviteForm::addGroupInvite(const GroupInvite& inviteInfo)
{
    // supress duplicate invite messages
    for (GroupInviteWidget* existing : invites) {
        if (existing->getInviteInfo().getInvite() == inviteInfo.getInvite()) {
            return false;
        }
    }

    GroupInviteWidget* widget = new GroupInviteWidget(this, inviteInfo, settings,
        core);
    scroll->widget()->layout()->addWidget(widget);
    invites.append(widget);
    connect(widget, &GroupInviteWidget::accepted, [this] (const GroupInvite& inviteInfo_) {
        deleteInviteWidget(inviteInfo_);
        emit groupInviteAccepted(inviteInfo_);
    });

    connect(widget, &GroupInviteWidget::rejected, [this] (const GroupInvite& inviteInfo_) {
        deleteInviteWidget(inviteInfo_);
    });
    if (isVisible()) {
        emit groupInvitesSeen();
        return false;
    }
    return true;
}

void GroupInviteForm::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    emit groupInvitesSeen();
}

/**
 * @brief Deletes accepted/declined group invite widget
 * @param inviteInfo Invite information of accepted/declined widget
 */
void GroupInviteForm::deleteInviteWidget(const GroupInvite& inviteInfo)
{
    auto deletingWidget =
        std::find_if(invites.begin(), invites.end(), [=](const GroupInviteWidget* widget) {
            return inviteInfo == widget->getInviteInfo();
        });
    (*deletingWidget)->deleteLater();
    scroll->widget()->layout()->removeWidget(*deletingWidget);
    invites.erase(deletingWidget);
}

void GroupInviteForm::retranslateUi()
{
    headLabel->setText(tr("Groups"));
    if (createButton) {
        createButton->setText(tr("Create new group"));
    }
    inviteBox->setTitle(tr("Group invites"));
    for (GroupInviteWidget* invite : invites) {
        invite->retranslateUi();
    }
}
