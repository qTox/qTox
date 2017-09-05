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

    CHANGED_SIGNAL_IMPL(QString, AboutFriend, name)
    CHANGED_SIGNAL_IMPL(QString, AboutFriend, status)
    CHANGED_SIGNAL_IMPL(QString, AboutFriend, publicKey)

    CHANGED_SIGNAL_IMPL(QPixmap, AboutFriend, avatar)
    CHANGED_SIGNAL_IMPL(QString, AboutFriend, note)

    CHANGED_SIGNAL_IMPL(QString, AboutFriend, autoAcceptDir)
    CHANGED_SIGNAL_IMPL(AutoAcceptCallFlags, AboutFriend, autoAcceptCall)
    CHANGED_SIGNAL_IMPL(bool, AboutFriend, autoGroupInvite)

signals:
    void nameChanged(const QString& name);
    void statusChanged(const QString& status);
    void publicKeyChanged(const QString& pk);

    void avatarChanged(const QPixmap& avatar);
    void noteChanged(const QString& val);

    void autoAcceptDirChanged(const QString& path);
    void autoAcceptCallChanged(const AutoAcceptCallFlags& flag);
    void autoGroupInviteChanged(const bool& enabled);

private:
    const Friend* const f;
};

#endif // ABOUT_FRIEND_H
