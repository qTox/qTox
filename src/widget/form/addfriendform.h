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

#pragma once

#include "src/core/toxid.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSet>
#include <QTextEdit>
#include <QVBoxLayout>

class QTabWidget;

class ContentLayout;

class AddFriendForm : public QObject
{
    Q_OBJECT
public:
    enum Mode
    {
        AddFriend = 0,
        ImportContacts = 1,
        FriendRequest = 2
    };

    AddFriendForm();
    AddFriendForm(const AddFriendForm&) = delete;
    AddFriendForm& operator=(const AddFriendForm&) = delete;
    ~AddFriendForm();

    bool isShown() const;
    void show(ContentLayout* contentLayout);
    void setMode(Mode mode);

    bool addFriendRequest(const QString& friendAddress, const QString& message);

signals:
    void friendRequested(const ToxId& friendAddress, const QString& message);
    void friendRequestAccepted(const ToxPk& friendAddress);
    void friendRequestsSeen();

public slots:
    void onUsernameSet(const QString& userName);

private slots:
    void onSendTriggered();
    void onIdChanged(const QString& id);
    void onImportSendClicked();
    void onImportOpenClicked();
    void onFriendRequestAccepted();
    void onFriendRequestRejected();
    void onCurrentChanged(int index);

private:
    void addFriend(const QString& idText);
    void retranslateUi();
    void addFriendRequestWidget(const QString& friendAddress, const QString& message);
    void removeFriendRequestWidget(QWidget* friendWidget);
    void retranslateAcceptButton(QPushButton* acceptButton);
    void retranslateRejectButton(QPushButton* rejectButton);
    void deleteFriendRequest(const ToxId& toxId);
    void setIdFromClipboard();
    QString getMessage() const;
    QString getImportMessage() const;

private:
    QLabel headLabel;
    QLabel toxIdLabel;
    QLabel messageLabel;
    QLabel importFileLabel;
    QLabel importMessageLabel;

    QPushButton sendButton;
    QPushButton importFileButton;
    QPushButton importSendButton;
    QLineEdit toxId;
    QTextEdit message;
    QTextEdit importMessage;
    QVBoxLayout layout;
    QVBoxLayout headLayout;
    QVBoxLayout importContactsLayout;
    QHBoxLayout importFileLine;
    QWidget* head;
    QWidget* main;
    QWidget* importContacts;
    QString lastUsername;
    QTabWidget* tabWidget;
    QVBoxLayout* requestsLayout;
    QList<QPushButton*> acceptButtons;
    QList<QPushButton*> rejectButtons;
    QList<QString> contactsToImport;
};
