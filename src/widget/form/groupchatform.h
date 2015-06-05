/*
    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
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
    GroupChatForm(Group* chatGroup);
    ~GroupChatForm();

    void onUserListChanged();
    void peerAudioPlaying(int peer);

    void keyPressEvent(QKeyEvent* ev);
    void keyReleaseEvent(QKeyEvent* ev);

signals:
    void groupTitleChanged(int groupnum, const QString& name);

private slots:
    void onSendTriggered();
    void onMicMuteToggle();
    void onVolMuteToggle();
    void onCallClicked();

protected:
    // drag & drop
    void dragEnterEvent(QDragEnterEvent* ev);
    void dropEvent(QDropEvent* ev);

private:
    void retranslateUi();

private:
    Group* group;
    QList<QLabel*> peerLabels; // maps peernumbers to the QLabels in namesListLayout
    QMap<int, QTimer*> peerAudioTimers; // timeout = peer stopped sending audio
    FlowLayout* namesListLayout;
    QLabel *nusersLabel;
    TabCompleter* tabber;
    bool inCall;
};

#endif // GROUPCHATFORM_H
