/*
    Copyright © 2017-2019 by The qTox Project Contributors

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

#include "groupinvitewidget.h"

#include "src/core/core.h"
#include "src/nexus.h"
#include "src/persistence/settings.h"
#include "src/widget/tool/croppinglabel.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QSignalMapper>

/**
 * @class GroupInviteWidget
 *
 * @brief This class shows information about single group invite
 * and provides buttons to accept/reject it
 */

GroupInviteWidget::GroupInviteWidget(QWidget* parent, const GroupInvite& invite,
    Settings& settings_, Core& core_)
    : QWidget(parent)
    , acceptButton(new QPushButton(this))
    , rejectButton(new QPushButton(this))
    , inviteMessageLabel(new CroppingLabel(this))
    , widgetLayout(new QHBoxLayout(this))
    , inviteInfo(invite)
    , settings{settings_}
    , core{core_}
{
    connect(acceptButton, &QPushButton::clicked, [=]() { emit accepted(inviteInfo); });
    connect(rejectButton, &QPushButton::clicked, [=]() { emit rejected(inviteInfo); });
    widgetLayout->addWidget(inviteMessageLabel);
    widgetLayout->addWidget(acceptButton);
    widgetLayout->addWidget(rejectButton);
    setLayout(widgetLayout);
    retranslateUi();
}

/**
 * @brief Retranslate all elements in the form.
 */
void GroupInviteWidget::retranslateUi()
{
    QString name = core.getFriendUsername(inviteInfo.getFriendId());
    QDateTime inviteDate = inviteInfo.getInviteDate();
    QString date = inviteDate.toString(settings.getDateFormat());
    QString time = inviteDate.toString(settings.getTimestampFormat());

    inviteMessageLabel->setText(
        tr("Invited by %1 on %2 at %3.").arg("<b>%1</b>").arg(name.toHtmlEscaped(), date, time));
    acceptButton->setText(tr("Join"));
    rejectButton->setText(tr("Decline"));
}

/**
 * @brief Returns infomation about invitation - e.g., who and when sent
 * @return Invite information object
 */
const GroupInvite GroupInviteWidget::getInviteInfo() const
{
    return inviteInfo;
}
