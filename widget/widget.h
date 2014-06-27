#ifndef WIDGET_H
#define WIDGET_H

#include <QThread>
#include <QWidget>
#include <QString>
#include "core.h"
#include "widget/form/addfriendform.h"
#include "widget/form/settingsform.h"

namespace Ui {
class Widget;
}

class GroupWidget;
class AddFriendForm;
class SettingsForm;
class FriendWidget;
class Group;

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    QString getUsername();
    Core* getCore();
    static Widget* getInstance();
    ~Widget();

signals:
    void friendRequestAccepted(const QString& userId);
    void friendRequested(const QString& friendAddress, const QString& message);
    void statusSet(Status status);
    void statusSelected(Status status);
    void usernameChanged(const QString& username);
    void statusMessageChanged(const QString& statusMessage);

private slots:
    void onConnected();
    void onDisconnected();
    void onStatusSet(Status status);
    void onAddClicked();
    void onGroupClicked();
    void onTransferClicked();
    void onSettingsClicked();
    void onFailedToStartCore();
    void onUsernameChanged(const QString& newUsername, const QString& oldUsername);
    void onStatusMessageChanged(const QString& newStatusMessage, const QString& oldStatusMessage);
    void onUsernameChanged(const QString& newUsername);
    void onStatusMessageChanged(const QString& newStatusMessage);
    void setUsername(const QString& username);
    void setStatusMessage(const QString &statusMessage);
    void addFriend(int friendId, const QString& userId);
    void addFriendFailed(const QString& userId);
    void onFriendStatusChanged(int friendId, Status status);
    void onFriendStatusMessageChanged(int friendId, const QString& message);
    void onFriendUsernameChanged(int friendId, const QString& username);
    void onFriendStatusMessageLoaded(int friendId, const QString& message);
    void onFriendUsernameLoaded(int friendId, const QString& username);
    void onFriendWidgetClicked(FriendWidget* widget);
    void onFriendMessageReceived(int friendId, const QString& message);
    void onFriendRequestReceived(const QString& userId, const QString& message);
    void onGroupInviteReceived(int friendId, uint8_t *publicKey);
    void onGroupMessageReceived(int groupnumber, int friendgroupnumber, const QString& message);
    void onGroupNamelistChanged(int groupnumber, int peernumber, uint8_t change);
    void onGroupWidgetClicked(GroupWidget* widget);
    void removeFriend(int friendId);
    void removeGroup(int groupId);

private:
    void hideMainForms();
    Group* createGroup(int groupId);

private:
    Ui::Widget *ui;
    Core* core;
    QThread* coreThread;
    AddFriendForm friendForm;
    SettingsForm settingsForm;
    static Widget* instance;
    FriendWidget* activeFriendWidget;
};

#endif // WIDGET_H
