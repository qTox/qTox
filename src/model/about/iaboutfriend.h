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

    virtual QString getName() const = 0;
    virtual QString getStatusMessage() const = 0;
    virtual QString getPublicKey() const = 0;

    virtual QPixmap getAvatar() const = 0;

    virtual QString getNote() const = 0;
    virtual void setNote(const QString& note) = 0;

    virtual QString getAutoAcceptDir() const = 0;
    virtual void setAutoAcceptDir(const QString& path) = 0;

    virtual AutoAcceptCall getAutoAcceptCall() const = 0;
    virtual void setAutoAcceptCall(AutoAcceptCall flag) = 0;

    virtual bool getAutoGroupInvite() const = 0;
    virtual void setAutoGroupInvite(bool enabled) = 0;

    virtual bool clearHistory() = 0;

    /* signals */
    CHANGED_SIGNAL(QString, name);
    CHANGED_SIGNAL(QString, status);
    CHANGED_SIGNAL(QString, publicKey);

    CHANGED_SIGNAL(QPixmap, avatar);
    CHANGED_SIGNAL(QString, note);

    CHANGED_SIGNAL(QString, autoAcceptDir);
    CHANGED_SIGNAL(AutoAcceptCall, autoAcceptCall);
    CHANGED_SIGNAL(bool, autoGroupInvite);
};

#endif // I_ABOUT_FRIEND_H
