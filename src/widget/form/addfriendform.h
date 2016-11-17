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

#ifndef ADDFRIENDFORM_H
#define ADDFRIENDFORM_H

#include <src/widget/contentwidget.h>

#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QSet>
#include <QVBoxLayout>

class QTabWidget;

class AddFriendForm : public ContentWidget
{
    Q_OBJECT
public:
    enum Mode
    {
        AddFriend = 0,
        FriendRequest = 1,
        GroupInvite = 2
    };

    AddFriendForm(QWidget* parent = nullptr);
    AddFriendForm(const AddFriendForm&) = delete;
    AddFriendForm& operator=(const AddFriendForm&) = delete;
    ~AddFriendForm();

    bool isShown() const;
    QString getMessage() const;
    void setMode(Mode mode);

    bool addFriendRequest(const QString& friendAddress, const QString& message);

signals:
    void friendRequested(const QString& friendAddress, const QString& message);
    void friendRequestAccepted(const QString& friendAddress);
    void friendRequestsSeen();

public slots:
    void onUsernameSet(const QString& userName);

private slots:
    void onSendTriggered();
    void onIdChanged(const QString &id);
    void onFriendRequestAccepted();
    void onFriendRequestRejected();
    void onCurrentChanged(int index);

private:
    void retranslateUi();
    void addFriendRequestWidget(const QString& friendAddress, const QString& message);
    void removeFriendRequestWidget(QWidget *friendWidget);
    void retranslateAcceptButton(QPushButton* acceptButton);
    void retranslateRejectButton(QPushButton* rejectButton);
    void deleteFriendRequest(const QString &toxId);
    void setIdFromClipboard();

private:
    QLabel headLabel, toxIdLabel, messageLabel;
    QPushButton sendButton;
    QLineEdit toxId;
    QTextEdit message;
    QWidget *head, *main;
    QString lastUsername;
    QTabWidget* tabWidget;
    QVBoxLayout* requestsLayout;
    QList<QPushButton*> acceptButtons;
    QList<QPushButton*> rejectButtons;
};

#endif // ADDFRIENDFORM_H
