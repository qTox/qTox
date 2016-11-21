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

#ifndef GROUPCHATFORM_H
#define GROUPCHATFORM_H

#include "genericchatform.h"
#include <QMap>

namespace Ui {class MainWindow;}
class Group;
class TabCompleter;
class FlowLayout;
class QTimer;

class GroupChatForm : public GenericChatForm
{
    Q_OBJECT
public:
    explicit GroupChatForm(Group* chatGroup, QWidget* parent = nullptr);
    ~GroupChatForm();


private slots:
    void onUserListChanged(const Group& g, int numPeers, quint8 change);
    void onGroupTitleChanged(int groupId, const QString& title,
                             const QString& author);
    void onPeerAudioPlaying(int groupId, int peer);
    void onSendTriggered();
    void onMicMuteToggle();
    void onVolMuteToggle();
    void onCallButtonClicked();
    void onMessageReceived(int groupId, int peerNo, const QString& message,
                           bool isAction);
    void onSendResult(int groupId, const QString& message, int result);

private:
    void updateCallButtons();
    void updateMuteMicButton();
    void updateMuteVolButton();

    GenericNetCamView* createNetcam() final;
    void keyPressEvent(QKeyEvent* ev) final;
    void keyReleaseEvent(QKeyEvent* ev) final;
    void dragEnterEvent(QDragEnterEvent* ev) final;
    void dropEvent(QDropEvent* ev) final;

private:
    void retranslateUi();

private:
    // TODO: flags are deprecated -> remove
    bool audioInputFlag;
    bool audioOutputFlag;

    Group* group;
    QList<QLabel*> peerLabels;
    QMap<int, QTimer*> peerAudioTimers;
    FlowLayout* namesListLayout;
    QLabel *nusersLabel;
    TabCompleter* tabber;
    bool inCall;
    QString correctNames(QString& name);
};

#endif // GROUPCHATFORM_H
