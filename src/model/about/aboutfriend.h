#ifndef ABOUT_FRIEND_H
#define ABOUT_FRIEND_H

#include "iaboutfriend.h"

#include "src/persistence/ifriendsettings.h"

#include <QObject>

class Friend;
class IFriendSettings;

class AboutFriend : public IAboutFriend
{
    Q_OBJECT

public:
    AboutFriend(const Friend* f, IFriendSettings* const settings);

    QString getName() const override;
    QString getStatusMessage() const override;
    ToxPk getPublicKey() const override;

    QPixmap getAvatar() const override;

    QString getNote() const override;
    void setNote(const QString& note) override;

    QString getAutoAcceptDir() const override;
    void setAutoAcceptDir(const QString& path) override;

    IFriendSettings::AutoAcceptCallFlags getAutoAcceptCall() const override;
    void setAutoAcceptCall(IFriendSettings::AutoAcceptCallFlags flag) override;

    bool getAutoGroupInvite() const override;
    void setAutoGroupInvite(bool enabled) override;

    bool clearHistory() override;

    SIGNAL_IMPL(AboutFriend, nameChanged, const QString&)
    SIGNAL_IMPL(AboutFriend, statusChanged, const QString&)
    SIGNAL_IMPL(AboutFriend, publicKeyChanged, const QString&)

    SIGNAL_IMPL(AboutFriend, avatarChanged, const QPixmap&)
    SIGNAL_IMPL(AboutFriend, noteChanged, const QString&)

    SIGNAL_IMPL(AboutFriend, autoAcceptDirChanged, const QString&)
    SIGNAL_IMPL(AboutFriend, autoAcceptCallChanged, IFriendSettings::AutoAcceptCallFlags)
    SIGNAL_IMPL(AboutFriend, autoGroupInviteChanged, bool)

private:
    const Friend* const f;
    IFriendSettings* const settings;
};

#endif // ABOUT_FRIEND_H
