#ifndef I_ABOUT_FRIEND_H
#define I_ABOUT_FRIEND_H

#include "src/model/interface.h"
#include <QObject>

class IAboutFriend : public QObject
{
    Q_OBJECT

public:
    enum class AutoAcceptCall
    {
        None = 0x00,
        Audio = 0x01,
        Video = 0x02,
        AV = Audio | Video
    };
    using AutoAcceptCallFlags = QFlags<AutoAcceptCall>;

    virtual QString getName() const = 0;
    virtual QString getStatusMessage() const = 0;
    virtual QString getPublicKey() const = 0;

    virtual QPixmap getAvatar() const = 0;

    virtual QString getNote() const = 0;
    virtual void setNote(const QString& note) = 0;

    virtual QString getAutoAcceptDir() const = 0;
    virtual void setAutoAcceptDir(const QString& path) = 0;

    virtual AutoAcceptCallFlags getAutoAcceptCall() const = 0;
    virtual void setAutoAcceptCall(AutoAcceptCallFlags flag) = 0;

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
    DECLARE_SIGNAL(autoAcceptCallChanged, AutoAcceptCallFlags);
    DECLARE_SIGNAL(autoGroupInviteChanged, bool);
};

#endif // I_ABOUT_FRIEND_H
