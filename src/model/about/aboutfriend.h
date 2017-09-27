#ifndef ABOUT_FRIEND_H
#define ABOUT_FRIEND_H

#include "iaboutfriend.h"

#include <QObject>

class Friend;

class AboutFriend : public IAboutFriend
{
    Q_OBJECT

public:
    explicit AboutFriend(const Friend* f);

    QString getName() const override;
    QString getStatusMessage() const override;
    QString getPublicKey() const override;

    QPixmap getAvatar() const override;

    QString getNote() const override;
    void setNote(const QString& note) override;

    QString getAutoAcceptDir() const override;
    void setAutoAcceptDir(const QString& path) override;

    AutoAcceptCallFlags getAutoAcceptCall() const override;
    void setAutoAcceptCall(AutoAcceptCallFlags flag) override;

    bool getAutoGroupInvite() const override;
    void setAutoGroupInvite(bool enabled) override;

    bool clearHistory() override;

    SIGNAL_IMPL(AboutFriend, nameChanged, const QString&)
    SIGNAL_IMPL(AboutFriend, statusChanged, const QString&)
    SIGNAL_IMPL(AboutFriend, publicKeyChanged, const QString&)

    SIGNAL_IMPL(AboutFriend, avatarChanged, const QPixmap&)
    SIGNAL_IMPL(AboutFriend, noteChanged, const QString&)

    SIGNAL_IMPL(AboutFriend, autoAcceptDirChanged, const QString&)
    SIGNAL_IMPL(AboutFriend, autoAcceptCallChanged, AutoAcceptCallFlags)
    SIGNAL_IMPL(AboutFriend, autoGroupInviteChanged, bool)

private:
    const Friend* const f;
};

#endif // ABOUT_FRIEND_H
