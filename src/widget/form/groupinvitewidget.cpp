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

GroupInviteWidget::GroupInviteWidget(QWidget* parent, const GroupInvite& invite)
    : QWidget(parent)
    , widgetLayout(new QHBoxLayout(this))
    , acceptButton(new QPushButton(this))
    , rejectButton(new QPushButton(this))
    , inviteMessageLabel(new CroppingLabel(this))
    , inviteInfo(invite)
{
    connect(acceptButton, &QPushButton::clicked, [=] () {
        emit accepted(inviteInfo);
    });
    connect(rejectButton, &QPushButton::clicked, [=] () {
        emit rejected(inviteInfo);
    });
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
    QString name = Nexus::getCore()->getFriendUsername(inviteInfo.getFriendId());
    QDateTime inviteDate = inviteInfo.getInviteDate();
    QString date = inviteDate.toString(Settings::getInstance().getDateFormat());
    QString time = inviteDate.toString(Settings::getInstance().getTimestampFormat());

    inviteMessageLabel->setText(tr("Invited by %1 on %2 at %3.")
                                .arg("<b>%1</b>")
                                .arg(name.toHtmlEscaped(), date, time));
    acceptButton->setText(tr("Join"));
    rejectButton->setText(tr("Decline"));
}

/**
 * @brief Returns invite information object
 * @return Returns invite information object
 */
const GroupInvite GroupInviteWidget::getInviteInfo() const
{
    return inviteInfo;
}
