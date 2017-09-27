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
    CHANGED_SIGNAL(name, const QString&);
    CHANGED_SIGNAL(status, const QString&);
    CHANGED_SIGNAL(publicKey, const QString&);

    CHANGED_SIGNAL(avatar, const QPixmap&);
    CHANGED_SIGNAL(note, const QString&);

    CHANGED_SIGNAL(autoAcceptDir, const QString&);
    CHANGED_SIGNAL(autoAcceptCall, AutoAcceptCallFlags);
    CHANGED_SIGNAL(autoGroupInvite, bool);
};

#endif // I_ABOUT_FRIEND_H
