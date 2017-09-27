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

    CHANGED_SIGNAL_IMPL(AboutFriend, name, const QString&)
    CHANGED_SIGNAL_IMPL(AboutFriend, status, const QString&)
    CHANGED_SIGNAL_IMPL(AboutFriend, publicKey, const QString&)

    CHANGED_SIGNAL_IMPL(AboutFriend, avatar, const QPixmap&)
    CHANGED_SIGNAL_IMPL(AboutFriend, note, const QString&)

    CHANGED_SIGNAL_IMPL(AboutFriend, autoAcceptDir, const QString&)
    CHANGED_SIGNAL_IMPL(AboutFriend, autoAcceptCall, AutoAcceptCallFlags)
    CHANGED_SIGNAL_IMPL(AboutFriend, autoGroupInvite, bool)

private:
    const Friend* const f;
};

#endif // ABOUT_FRIEND_H
