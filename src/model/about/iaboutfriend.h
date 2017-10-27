#ifndef I_ABOUT_FRIEND_H
#define I_ABOUT_FRIEND_H

#include "src/model/interface.h"
#include "src/persistence/ifriendsettings.h"
#include <QObject>

class IAboutFriend : public QObject
{
    Q_OBJECT

public:
    virtual QString getName() const = 0;
    virtual QString getStatusMessage() const = 0;
    virtual ToxPk getPublicKey() const = 0;

    virtual QPixmap getAvatar() const = 0;

    virtual QString getNote() const = 0;
    virtual void setNote(const QString& note) = 0;

    virtual QString getAutoAcceptDir() const = 0;
    virtual void setAutoAcceptDir(const QString& path) = 0;

    virtual IFriendSettings::AutoAcceptCallFlags getAutoAcceptCall() const = 0;
    virtual void setAutoAcceptCall(IFriendSettings::AutoAcceptCallFlags flag) = 0;

    virtual bool getAutoGroupInvite() const = 0;
    virtual void setAutoGroupInvite(bool enabled) = 0;

    virtual bool clearHistory() = 0;

    /* signals */
    DECLARE_SIGNAL(nameChanged, const QString&);
    DECLARE_SIGNAL(statusChanged, const QString&);
    DECLARE_SIGNAL(publicKeyChanged, const QString&);

    DECLARE_SIGNAL(avatarChanged, const QPixmap&);
    DECLARE_SIGNAL(noteChanged, const QString&);

    DECLARE_SIGNAL(autoAcceptDirChanged, const QString&);
    DECLARE_SIGNAL(autoAcceptCallChanged, IFriendSettings::AutoAcceptCallFlags);
    DECLARE_SIGNAL(autoGroupInviteChanged, bool);
};

#endif // I_ABOUT_FRIEND_H
