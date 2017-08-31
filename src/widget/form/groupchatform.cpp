/*
    Copyright © 2014-2015 by The qTox Project Contributors

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

#include "groupchatform.h"

#include "tabcompleter.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/model/friend.h"
#include "src/friendlist.h"
#include "src/model/group.h"
#include "src/video/groupnetcamview.h"
#include "src/widget/flowlayout.h"
#include "src/widget/form/chatform.h"
#include "src/widget/groupwidget.h"
#include "src/widget/maskablepixmapwidget.h"
#include "src/widget/style.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/widget/translator.h"

#include <QDragEnterEvent>
#include <QMimeData>
#include <QRegularExpression>
#include <QTimer>
#include <QToolButton>

/**
 * @brief Edit name for correct representation if it is needed
 * @param name Editing string
 * @return Source name if it does not contain any newline character, otherwise it chops characters
 * starting with first newline character and appends "..."
 */
QString editName(const QString& name)
{
    const int pos = name.indexOf(QRegularExpression(QStringLiteral("[\n\r]")));
    if (pos == -1) {
        return name;
    }

    QString result = name;
    const int len = result.length();
    result.chop(len - pos);
    result.append(QStringLiteral("…")); // \u2026 Unicode symbol, not just three separate dots
    return result;
}

/**
 * @var QList<QLabel*> GroupChatForm::peerLabels
 * @brief Maps peernumbers to the QLabels in namesListLayout.
 *
 * @var QMap<int, QTimer*> GroupChatForm::peerAudioTimers
 * @brief Timeout = peer stopped sending audio.
 */

GroupChatForm::GroupChatForm(Group* chatGroup)
    : group(chatGroup)
    , inCall(false)
{
    nusersLabel = new QLabel();

    tabber = new TabCompleter(msgEdit, group);

    fileButton->setEnabled(false);
    if (group->isAvGroupchat()) {
        videoButton->setEnabled(false);
        videoButton->setObjectName("grey");
    } else {
        videoButton->setVisible(false);
        callButton->setVisible(false);
        volButton->setVisible(false);
        micButton->setVisible(false);
    }

    nameLabel->setText(group->getName());

    nusersLabel->setFont(Style::getFont(Style::Medium));
    nusersLabel->setObjectName("statusLabel");
    retranslateUi();

    avatar->setPixmap(Style::scaleSvgImage(":/img/group_dark.svg", avatar->width(), avatar->height()));

    msgEdit->setObjectName("group");

    namesListLayout = new FlowLayout(0, 5, 0);
    headTextLayout->addWidget(nusersLabel);
    headTextLayout->addLayout(namesListLayout);
    headTextLayout->addStretch();

    nameLabel->setMinimumHeight(12);
    nusersLabel->setMinimumHeight(12);

    connect(sendButton, SIGNAL(clicked()), this, SLOT(onSendTriggered()));
    connect(msgEdit, SIGNAL(enterPressed()), this, SLOT(onSendTriggered()));
    connect(msgEdit, &ChatTextEdit::tabPressed, tabber, &TabCompleter::complete);
    connect(msgEdit, &ChatTextEdit::keyPressed, tabber, &TabCompleter::reset);
    connect(callButton, &QPushButton::clicked, this, &GroupChatForm::onCallClicked);
    connect(micButton, SIGNAL(clicked()), this, SLOT(onMicMuteToggle()));
    connect(volButton, SIGNAL(clicked()), this, SLOT(onVolMuteToggle()));
    connect(nameLabel, &CroppingLabel::editFinished, this, [=](const QString& newName) {
        if (!newName.isEmpty()) {
            nameLabel->setText(newName);
            chatGroup->setName(newName);
        }
    });
    connect(group, &Group::userListChanged, this, &GroupChatForm::onUserListChanged);

    setAcceptDrops(true);
    Translator::registerHandler(std::bind(&GroupChatForm::retranslateUi, this), this);
}

GroupChatForm::~GroupChatForm()
{
    Translator::unregister(this);
}

void GroupChatForm::onSendTriggered()
{
    QString msg = msgEdit->toPlainText();
    if (msg.isEmpty())
        return;

    msgEdit->setLastMessage(msg);
    msgEdit->clear();

    if (group->getPeersCount() != 1) {
        if (msg.startsWith(ChatForm::ACTION_PREFIX, Qt::CaseInsensitive)) {
            msg.remove(0, ChatForm::ACTION_PREFIX.length());
            emit sendAction(group->getId(), msg);
        } else {
            emit sendMessage(group->getId(), msg);
        }
    } else {
        if (msg.startsWith(ChatForm::ACTION_PREFIX, Qt::CaseInsensitive))
            addSelfMessage(msg.mid(ChatForm::ACTION_PREFIX.length()), QDateTime::currentDateTime(),
                           true);
        else
            addSelfMessage(msg, QDateTime::currentDateTime(), false);
    }
}

/**
 * @brief This slot is intended to connect to Group::userListChanged signal.
 * Brief list of actions made by slot:
 *      1) sets text of how many people are in the group;
 *      2) creates lexicographically sorted comma-separated list of user names, each name in its own
 *      label;
 *      3) sets call button style depending on peer count and etc.
 */
void GroupChatForm::onUserListChanged()
{
    updateUserCount();
    updateUserNames();

    // Enable or disable call button
    const int peersCount = group->getPeersCount();
    if (peersCount > 1 && group->isAvGroupchat()) {
        // don't set button to green if call running
        if (!inCall) {
            callButton->setEnabled(true);
            callButton->setObjectName("green");
            callButton->style()->polish(callButton);
            callButton->setToolTip(tr("Start audio call"));
        }
    } else {
        callButton->setEnabled(false);
        callButton->setObjectName("grey");
        callButton->style()->polish(callButton);
        callButton->setToolTip("");
        Core::getInstance()->getAv()->leaveGroupCall(group->getId());
        hideNetcam();
    }
}

/**
 * @brief Updates user names' labels at the top of group chat
 */
void GroupChatForm::updateUserNames()
{
    QLayoutItem* child;
    while ((child = namesListLayout->takeAt(0))) {
        child->widget()->hide();
        delete child->widget();
        delete child;
    }

    peerLabels.clear();
    const int peersCount = group->getPeersCount();
    peerLabels.reserve(peersCount);
    QVector<QLabel*> nickLabelList(peersCount);

    /* the list needs peers in peernumber order, nameLayout needs alphabetical
     * first traverse in peer number order, storing the QLabels as necessary */
    const QStringList names = group->getPeerList();
    int peerNumber = 0;
    for (const QString& fullName : names) {
        const QString editedName = editName(fullName).append(QLatin1String(", "));
        QLabel* const label = new QLabel(editedName);
        if (editedName != fullName) {
            label->setToolTip(fullName);
        }
        label->setTextFormat(Qt::PlainText);
        if (group->isSelfPeerNumber(peerNumber)) {
            label->setStyleSheet(QStringLiteral("QLabel {color : green;}"));
        } else if (netcam != nullptr) {
            static_cast<GroupNetCamView*>(netcam)->addPeer(peerNumber, fullName);
        }
        peerLabels.append(label);
        nickLabelList[peerNumber++] = label;
    }

    if (netcam != nullptr) {
        static_cast<GroupNetCamView*>(netcam)->clearPeers();
    }

    qSort(nickLabelList.begin(), nickLabelList.end(), [](const QLabel* a, const QLabel* b)
    {
        return a->text().toLower() < b->text().toLower();
    });
    // remove comma from last sorted label
    QLabel* const lastLabel = nickLabelList.last();
    QString labelText = lastLabel->text();
    labelText.chop(2);
    lastLabel->setText(labelText);
    for (QLabel* l : nickLabelList) {
        namesListLayout->addWidget(l);
    }
}

void GroupChatForm::peerAudioPlaying(int peer)
{
    peerLabels[peer]->setStyleSheet(QStringLiteral("QLabel {color : red;}"));
    if (!peerAudioTimers[peer]) {
        peerAudioTimers[peer] = new QTimer(this);
        peerAudioTimers[peer]->setSingleShot(true);
        connect(peerAudioTimers[peer], &QTimer::timeout, [this, peer] {
            if (netcam)
                static_cast<GroupNetCamView*>(netcam)->removePeer(peer);

            if (peer >= peerLabels.size())
                return;

            peerLabels[peer]->setStyleSheet("");
            delete peerAudioTimers[peer];
            peerAudioTimers[peer] = nullptr;
        });

        if (netcam) {
            static_cast<GroupNetCamView*>(netcam)->removePeer(peer);
            static_cast<GroupNetCamView*>(netcam)->addPeer(peer, group->getPeerList()[peer]);
        }
    }
    peerAudioTimers[peer]->start(500);
}

void GroupChatForm::dragEnterEvent(QDragEnterEvent* ev)
{
    ToxId toxId = ToxId(ev->mimeData()->text());
    Friend* frnd = FriendList::findFriend(toxId.getPublicKey());
    if (frnd)
        ev->acceptProposedAction();
}

void GroupChatForm::dropEvent(QDropEvent* ev)
{
    ToxId toxId = ToxId(ev->mimeData()->text());
    Friend* frnd = FriendList::findFriend(toxId.getPublicKey());
    if (!frnd)
        return;

    int friendId = frnd->getId();
    int groupId = group->getId();
    if (frnd->getStatus() != Status::Offline) {
        Core::getInstance()->groupInviteFriend(friendId, groupId);
    }
}

void GroupChatForm::onMicMuteToggle()
{
    if (audioInputFlag) {
        CoreAV* av = Core::getInstance()->getAv();
        if (micButton->objectName() == "red") {
            av->muteCallInput(group, false);
            micButton->setObjectName("green");
            micButton->setToolTip(tr("Mute microphone"));
        } else {
            av->muteCallInput(group, true);
            micButton->setObjectName("red");
            micButton->setToolTip(tr("Unmute microphone"));
        }

        Style::repolish(micButton);
    }
}

void GroupChatForm::onVolMuteToggle()
{
    if (audioOutputFlag) {
        CoreAV* av = Core::getInstance()->getAv();
        if (volButton->objectName() == "red") {
            av->muteCallOutput(group, false);
            volButton->setObjectName("green");
            volButton->setToolTip(tr("Mute call"));
        } else {
            av->muteCallOutput(group, true);
            volButton->setObjectName("red");
            volButton->setToolTip(tr("Unmute call"));
        }

        Style::repolish(volButton);
    }
}

void GroupChatForm::onCallClicked()
{
    if (!inCall) {
        Core::getInstance()->getAv()->joinGroupCall(group->getId());
        audioInputFlag = true;
        audioOutputFlag = true;
        callButton->setObjectName("red");
        callButton->style()->polish(callButton);
        callButton->setToolTip(tr("End audio call"));
        micButton->setObjectName("green");
        micButton->style()->polish(micButton);
        micButton->setToolTip(tr("Mute microphone"));
        volButton->setObjectName("green");
        volButton->style()->polish(volButton);
        volButton->setToolTip(tr("Mute call"));
        inCall = true;
        showNetcam();
    } else {
        Core::getInstance()->getAv()->leaveGroupCall(group->getId());
        audioInputFlag = false;
        audioOutputFlag = false;
        callButton->setObjectName("green");
        callButton->style()->polish(callButton);
        callButton->setToolTip(tr("Start audio call"));
        micButton->setObjectName("grey");
        micButton->style()->polish(micButton);
        micButton->setToolTip("");
        volButton->setObjectName("grey");
        volButton->style()->polish(volButton);
        volButton->setToolTip("");
        inCall = false;
        hideNetcam();
    }
}

GenericNetCamView* GroupChatForm::createNetcam()
{
    GroupNetCamView* view = new GroupNetCamView(group->getId(), this);

    QStringList names = group->getPeerList();
    for (int i = 0; i < names.size(); ++i) {
        if (!group->isSelfPeerNumber(i))
            static_cast<GroupNetCamView*>(view)->addPeer(i, names[i]);
    }

    return view;
}

void GroupChatForm::keyPressEvent(QKeyEvent* ev)
{
    // Push to talk (CTRL+P)
    if (ev->key() == Qt::Key_P && (ev->modifiers() & Qt::ControlModifier) && inCall) {
        CoreAV* av = Core::getInstance()->getAv();
        if (!av->isGroupCallInputMuted(group)) {
            av->muteCallInput(group, false);
            micButton->setObjectName("green");
            micButton->style()->polish(micButton);
            Style::repolish(micButton);
        }
    }

    if (msgEdit->hasFocus())
        return;
}

void GroupChatForm::keyReleaseEvent(QKeyEvent* ev)
{
    // Push to talk (CTRL+P)
    if (ev->key() == Qt::Key_P && (ev->modifiers() & Qt::ControlModifier) && inCall) {
        CoreAV* av = Core::getInstance()->getAv();
        if (av->isGroupCallInputMuted(group)) {
            av->muteCallInput(group, true);
            micButton->setObjectName("red");
            micButton->style()->polish(micButton);
            Style::repolish(micButton);
        }
    }

    if (msgEdit->hasFocus())
        return;
}

/**
 * @brief Updates users' count label text
 */
void GroupChatForm::updateUserCount()
{
    const int peersCount = group->getPeersCount();
    if (peersCount == 1) {
        nusersLabel->setText(tr("1 user in chat", "Number of users in chat"));
    } else {
        nusersLabel->setText(tr("%1 users in chat", "Number of users in chat").arg(peersCount));
    }
}

void GroupChatForm::retranslateUi()
{
    updateUserCount();
}
