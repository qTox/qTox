/*
    Copyright © 2014-2015 by The qTox Project

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

#include <QDragEnterEvent>
#include <QMimeData>
#include <QTimer>

#include "tabcompleter.h"
#include "src/group.h"
#include "src/friend.h"
#include "src/widget/groupwidget.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/widget/maskablepixmapwidget.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/widget/style.h"
#include "src/widget/flowlayout.h"
#include "src/widget/translator.h"
#include "src/widget/form/chatform.h"
#include "src/video/groupnetcamview.h"

/**
 * @var QList<QLabel*> GroupChatForm::peerLabels
 * @brief Maps peernumbers to the QLabels in namesListLayout.
 *
 * @var QMap<int, QTimer*> GroupChatForm::peerAudioTimers
 * @brief Timeout = peer stopped sending audio.
 */

GroupChatForm::GroupChatForm(Group* chatGroup, QWidget* parent)
    : GenericChatForm(parent)
    , group(chatGroup)
{
    nusersLabel = new QLabel();

    tabber = new TabCompleter(msgEdit, group);

    fileButton->setEnabled(false);
    if (group->isAvGroupchat())
    {
        videoButton->setEnabled(false);
    }
    else
    {
        videoButton->setVisible(false);
        callButton->setVisible(false);
        volButton->setVisible(false);
        micButton->setVisible(false);
    }

    nameLabel->setText(group->getName());

    nusersLabel->setFont(Style::getFont(Style::Medium));
    nusersLabel->setObjectName("statusLabel");
    retranslateUi();

    avatar->setPixmap(Style::scaleSvgImage(":/img/group_dark.svg",
                                           avatar->width(), avatar->height()));

    msgEdit->setObjectName("group");

    namesListLayout = new FlowLayout(0,5,0);
    QStringList names(group->getPeerList());

    for (QString& name : names)
    {
        QLabel *l = new QLabel();
        QString tooltip = correctNames(name);
        if (tooltip.isNull())
        {
            l->setToolTip(tooltip);
        }
        l->setText(name);
        l->setTextFormat(Qt::PlainText);
        namesListLayout->addWidget(l);
    }

    headTextLayout->addWidget(nusersLabel);
    headTextLayout->addLayout(namesListLayout);
    headTextLayout->addStretch();

    nameLabel->setMinimumHeight(12);
    nusersLabel->setMinimumHeight(12);

    connect(sendButton, SIGNAL(clicked()), this, SLOT(onSendTriggered()));
    connect(msgEdit, SIGNAL(enterPressed()), this, SLOT(onSendTriggered()));
    connect(msgEdit, &ChatTextEdit::tabPressed, tabber, &TabCompleter::complete);
    connect(msgEdit, &ChatTextEdit::keyPressed, tabber, &TabCompleter::reset);
    connect(callButton, &QPushButton::clicked,
            this, &GroupChatForm::onCallButtonClicked);
    connect(micButton, SIGNAL(clicked()), this, SLOT(onMicMuteToggle()));
    connect(volButton, SIGNAL(clicked()), this, SLOT(onVolMuteToggle()));
    connect(nameLabel, &CroppingLabel::editFinished, this, [=](const QString& newName)
    {
        group->setName(newName);
    });

    const Core* core = Core::getInstance();
    connect(core, &Core::groupPeerAudioPlaying,
            this, &GroupChatForm::onPeerAudioPlaying);
    connect(core, &Core::groupTitleChanged,
            this, &GroupChatForm::onGroupTitleChanged);
    connect(Group::notify(), &GroupNotify::userListChanged,
            this, &GroupChatForm::onUserListChanged);
    connect(core, &Core::groupSentResult,
            this, &GroupChatForm::onSendResult);
    connect(core, &Core::groupMessageReceived,
            this, &GroupChatForm::onMessageReceived);

    setAcceptDrops(true);
    Translator::registerHandler(std::bind(&GroupChatForm::retranslateUi, this), this);

    updateCallButtons();
}

GroupChatForm::~GroupChatForm()
{
    Translator::unregister(this);
}

QString GroupChatForm::correctNames(QString& name)
{
    int pos = name.indexOf(QRegExp("\n|\r\n|\r"));
    int len = name.length();
    if ( (pos < len) && (pos !=-1) )
    {
        QString tooltip = name;
        name.remove( pos, len-pos );
        name.append("...");
        return tooltip;
    }
    else
    {
        return QString();
    }
}

void GroupChatForm::onSendTriggered()
{
    QString msg = msgEdit->toPlainText();
    if (msg.isEmpty())
        return;

    msgEdit->setLastMessage(msg);
    msgEdit->clear();

    if (group->getPeersCount() != 1)
    {
        if (msg.startsWith(ChatForm::ACTION_PREFIX, Qt::CaseInsensitive))
        {
            msg.remove(0, ChatForm::ACTION_PREFIX.length());
            Core::getInstance()->sendAction(group->getGroupId(), msg);
        }
        else
        {
            Core::getInstance()->sendMessage(group->getGroupId(), msg);
        }
    }
    else
    {
        if (msg.startsWith(ChatForm::ACTION_PREFIX, Qt::CaseInsensitive))
            addSelfMessage(msg.mid(ChatForm::ACTION_PREFIX.length()), true, QDateTime::currentDateTime(), true);
        else
            addSelfMessage(msg, false, QDateTime::currentDateTime(), true);
    }
}

void GroupChatForm::onUserListChanged(const Group& g, int numPeers,
                                      quint8 change)
{
    Q_UNUSED(change);

    if (g.getGroupId() == group->getGroupId())
        return;

    int n = numPeers;
    nusersLabel->setText(tr("%n user(s) in chat", "Number of users in chat", n));

    QLayoutItem *child;
    while ((child = namesListLayout->takeAt(0)))
    {
        child->widget()->hide();
        delete child->widget();
        delete child;
    }
    peerLabels.clear();

    // the list needs peers in peernumber order, nameLayout needs alphabetical
    QList<QLabel*> nickLabelList;

    // first traverse in peer number order, storing the QLabels as necessary
    QStringList names = group->getPeerList();
    unsigned nNames = names.size();
    for (unsigned i=0; i<nNames; ++i)
    {
        QString tooltip = correctNames(names[i]);
        peerLabels.append(new QLabel(names[i]));
        if (!tooltip.isEmpty())
            peerLabels[i]->setToolTip(tooltip);
        peerLabels[i]->setTextFormat(Qt::PlainText);
        nickLabelList.append(peerLabels[i]);
        if (group->isSelfPeerNumber(i))
            peerLabels[i]->setStyleSheet("QLabel {color : green;}");

        if (netcam && !group->isSelfPeerNumber(i))
            static_cast<GroupNetCamView*>(netcam.data())->addPeer(i, names[i]);
    }

    if (netcam)
        static_cast<GroupNetCamView*>(netcam.data())->clearPeers();

    // now alphabetize and add to layout
    qSort(nickLabelList.begin(), nickLabelList.end(), [](QLabel *a, QLabel *b){return a->text().toLower() < b->text().toLower();});
    for (unsigned i=0; i<nNames; ++i)
    {
        QLabel *label = nickLabelList.at(i);
        if (i != nNames - 1)
            label->setText(label->text() + ", ");

        namesListLayout->addWidget(label);
    }

    // Enable or disable call button
    if (group->isAvGroupchat() && numPeers < 1)
    {
        CoreAV* av = Core::getInstance()->getAv();
        av->leaveGroupCall(group->getGroupId());
        hideNetcam();
    }

    updateCallButtons();
}

void GroupChatForm::onPeerAudioPlaying(Group::ID groupId, int peerNo)
{
    if (groupId != group->getGroupId())
        return;

    peerLabels[peerNo]->setStyleSheet("QLabel {color : red;}");
    if (!peerAudioTimers[peerNo])
    {
        peerAudioTimers[peerNo] = new QTimer(this);
        peerAudioTimers[peerNo]->setSingleShot(true);
        connect(peerAudioTimers[peerNo], &QTimer::timeout, [this, peerNo]
        {
            if (netcam)
                static_cast<GroupNetCamView*>(netcam.data())->removePeer(peerNo);

            if (peerNo >= peerLabels.size())
                return;

            peerLabels[peerNo]->setStyleSheet("");
            delete peerAudioTimers[peerNo];
            peerAudioTimers[peerNo] = nullptr;
        });

        if (netcam)
        {
            GroupNetCamView* view = static_cast<GroupNetCamView*>(netcam.data());
            view->removePeer(peerNo);
            view->addPeer(peerNo, group->getPeerList()[peerNo]);
        }
    }
    peerAudioTimers[peerNo]->start(500);
}

void GroupChatForm::dragEnterEvent(QDragEnterEvent *ev)
{
    ToxId toxId = ToxId(ev->mimeData()->text());
    Friend frnd = Friend::get(toxId);
    if (frnd)
        ev->acceptProposedAction();
}

void GroupChatForm::dropEvent(QDropEvent *ev)
{
    ToxId toxId = ToxId(ev->mimeData()->text());
    Friend frnd = Friend::get(toxId);
    if (frnd)
    {
        Friend::ID friendId = frnd.getFriendId();
        int groupId = group->getGroupId();
        Core::getInstance()->groupInviteFriend(friendId, groupId);
    }
}

void GroupChatForm::onMicMuteToggle()
{
    CoreAV* av = Core::getInstance()->getAv();

    av->toggleMuteCallInput(group);
    updateMuteMicButton();
}

void GroupChatForm::onVolMuteToggle()
{
    CoreAV* av = Core::getInstance()->getAv();

    av->toggleMuteCallOutput(group);
    updateMuteVolButton();
}

void GroupChatForm::onCallButtonClicked()
{
    CoreAV* av = Core::getInstance()->getAv();
    if (av->isCallActive(group))
    {
        av->leaveGroupCall(group->getGroupId());
        hideNetcam();
    }
    else
    {
        av->joinGroupCall(group->getGroupId());
        callButton->setObjectName("yellow");
        callButton->setStyleSheet(Style::getStylesheet(QStringLiteral(":/ui/callButton/callButton.css")));
        callButton->setToolTip(tr("Cancel audio call"));
        showNetcam();
    }
}

void GroupChatForm::onGroupTitleChanged(Group::ID groupId, const QString& title,
                                        const QString& author)
{
    if (groupId == group->getGroupId())
    {
        setName(author);
        addSystemInfoMessage(tr("%1 has set the title to %2").arg(author)
                             .arg(title), ChatMessage::INFO,
                             QDateTime::currentDateTime());
    }
}

void GroupChatForm::onMessageReceived(int groupId, int peerNo,
                                      const QString& message, bool isAction)
{
    Q_UNUSED(peerNo);

    if (groupId != group->getGroupId())
        return;

    const Core* core = Core::getInstance();
    const QDateTime ts = QDateTime::currentDateTime();
    ToxId author = core->getGroupPeerToxId(groupId, peerNo);

    if (!isAction)
        addMessage(author, message, isAction, ts, true);
    else
        addAlertMessage(author, message, ts);
}

void GroupChatForm::onSendResult(Group::ID groupId, const QString& message,
                                 int result)
{
    Q_UNUSED(message);

    if (groupId != group->getGroupId())
        return;

    if (result == -1)
        addSystemInfoMessage(tr("Message failed to send"), ChatMessage::INFO,
                             QDateTime::currentDateTime());
}

void GroupChatForm::updateCallButtons()
{
    CoreAV* av = Core::getInstance()->getAv();
    if (av->isCallActive(group))
    {
        callButton->setObjectName("red");
        callButton->setToolTip(tr("End audio call"));

        if (av->isCallVideoEnabled(group))
        {
            videoButton->setObjectName("red");
            videoButton->setToolTip(tr("End video call"));
        }
    }
    else
    {
        bool avAvailable = group->isAvGroupchat() && group->getPeersCount() > 0;
        callButton->setEnabled(avAvailable);
        videoButton->setEnabled(avAvailable);

        if (callButton->isEnabled())
        {
            callButton->setObjectName("green");
            callButton->setToolTip(tr("Start audio call"));
        }
        else
        {
            callButton->setObjectName("");
            callButton->setToolTip("");
        }

        if (videoButton->isEnabled())
        {
            videoButton->setObjectName("green");
            videoButton->setToolTip(tr("Start video call"));
        }
        else
        {
            videoButton->setObjectName("");
            videoButton->setToolTip("");
        }
    }

    callButton->setStyleSheet(Style::getStylesheet(QStringLiteral(":/ui/callButton/callButton.css")));
    videoButton->setStyleSheet(Style::getStylesheet(QStringLiteral(":/ui/videoButton/videoButton.css")));

    updateMuteMicButton();
    updateMuteVolButton();
}

void GroupChatForm::updateMuteMicButton()
{
    const CoreAV* av = Core::getInstance()->getAv();

    micButton->setEnabled(av->isCallActive(group));

    if (micButton->isEnabled())
    {
        if (av->isCallInputMuted(group))
        {
            micButton->setObjectName("red");
            micButton->setToolTip(tr("Unmute microphone"));
        }
        else
        {
            micButton->setObjectName("green");
            micButton->setToolTip(tr("Mute microphone"));
        }
    }
    else
    {
        micButton->setToolTip("");
    }

    QString stylePath = QStringLiteral(":/ui/micButton/micButton.css");
    QString style = Style::getStylesheet(stylePath);
    micButton->setStyleSheet(style);
}

void GroupChatForm::updateMuteVolButton()
{
    const CoreAV* av = Core::getInstance()->getAv();

    volButton->setEnabled(av->isCallActive(group));

    if (volButton->isEnabled())
    {
        if (av->isCallOutputMuted(group))
        {
            volButton->setObjectName("red");
            volButton->setToolTip(tr("Unmute call"));
        }
        else
        {
            volButton->setObjectName("green");
            volButton->setToolTip(tr("Mute call"));
        }
    }
    else
    {
        volButton->setToolTip("");
    }

    QString stylePath = QStringLiteral(":/ui/volButton/volButton.css");
    QString style = Style::getStylesheet(stylePath);
    volButton->setStyleSheet(style);
}

GenericNetCamView *GroupChatForm::createNetcam()
{
    GroupNetCamView* view = new GroupNetCamView(group->getGroupId(), this);

    QStringList names = group->getPeerList();
    for (int i = 0; i<names.size(); ++i)
    {
        if (!group->isSelfPeerNumber(i))
            static_cast<GroupNetCamView*>(view)->addPeer(i, names[i]);
    }

    return view;
}

void GroupChatForm::keyPressEvent(QKeyEvent* ev)
{
    // Push to talk (CTRL+P)
    if (ev->key() == Qt::Key_P && (ev->modifiers() & Qt::ControlModifier))
    {
        CoreAV* av = Core::getInstance()->getAv();
        if (!av->isCallInputMuted(group))
        {
            av->toggleMuteCallInput(group);
            updateMuteMicButton();
        }
    }

    if (msgEdit->hasFocus())
        return;
}

void GroupChatForm::keyReleaseEvent(QKeyEvent* ev)
{
    // Push to talk (CTRL+P)
    if (ev->key() == Qt::Key_P && (ev->modifiers() & Qt::ControlModifier))
    {
        CoreAV* av = Core::getInstance()->getAv();
        if (av->isCallInputMuted(group))
        {
            av->isCallInputMuted(group);
            updateMuteMicButton();
        }
    }

    if (msgEdit->hasFocus())
        return;
}

void GroupChatForm::retranslateUi()
{
    int peersCount = group->getPeersCount();
    if (peersCount == 1)
        nusersLabel->setText(tr("1 user in chat", "Number of users in chat"));
    else
        nusersLabel->setText(tr("%1 users in chat", "Number of users in chat").arg(peersCount));
}
