/*
    Copyright © 2014-2019 by The qTox Project Contributors

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
#include "src/core/toxpk.h"
#include <QMap>

namespace Ui {
class MainWindow;
}
class Group;
class TabCompleter;
class FlowLayout;
class QTimer;
class GroupId;
class IMessageDispatcher;
class Message;

class GroupChatForm : public GenericChatForm
{
    Q_OBJECT
public:
    explicit GroupChatForm(Group* chatGroup, IChatLog& chatLog, IMessageDispatcher& messageDispatcher);
    ~GroupChatForm();

    void peerAudioPlaying(ToxPk peerPk);

private slots:
    void onScreenshotClicked() override;
    void onAttachClicked() override;
    void onMicMuteToggle();
    void onVolMuteToggle();
    void onCallClicked();
    void onUserJoined(const ToxPk& user, const QString& name);
    void onUserLeft(const ToxPk& user, const QString& name);
    void onPeerNameChanged(const ToxPk& peer, const QString& oldName, const QString& newName);
    void onTitleChanged(const QString& author, const QString& title);
    void onLabelContextMenuRequested(const QPoint& localPos);

protected:
    virtual GenericNetCamView* createNetcam() final override;
    virtual void keyPressEvent(QKeyEvent* ev) final override;
    virtual void keyReleaseEvent(QKeyEvent* ev) final override;
    // drag & drop
    virtual void dragEnterEvent(QDragEnterEvent* ev) final override;
    virtual void dropEvent(QDropEvent* ev) final override;

private:
    void retranslateUi();
    void updateUserCount(int numPeers);
    void updateUserNames();
    void leaveGroupCall();

private:
    Group* group;
    QMap<ToxPk, QLabel*> peerLabels;
    QMap<ToxPk, QTimer*> peerAudioTimers;
    FlowLayout* namesListLayout;
    QLabel* nusersLabel;
    TabCompleter* tabber;
    bool inCall;
};

#endif // GROUPCHATFORM_H
