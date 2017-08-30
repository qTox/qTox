#ifndef I_FRIEND_SETTINGS_H
#define I_FRIEND_SETTINGS_H

#include <QObject>
#include <QFlag>

class ToxPk;

class IFriendSettings : public QObject
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
    Q_DECLARE_FLAGS(AutoAcceptCallFlags, AutoAcceptCall)

    virtual QString getContactNote(const ToxPk& id) const = 0;
    virtual void setContactNote(const ToxPk& id, const QString& note) = 0;

    virtual QString getAutoAcceptDir(const ToxPk& id) const = 0;
    virtual void setAutoAcceptDir(const ToxPk& id, const QString& dir) = 0;

    virtual AutoAcceptCallFlags getAutoAcceptCall(const ToxPk& id) const = 0;
    virtual void setAutoAcceptCall(const ToxPk& id, AutoAcceptCallFlags accept) = 0;

    virtual bool getAutoGroupInvite(const ToxPk& id) const = 0;
    virtual void setAutoGroupInvite(const ToxPk& id, bool accept) = 0;

    virtual QString getFriendAlias(const ToxPk& id) const = 0;
    virtual void setFriendAlias(const ToxPk& id, const QString& alias) = 0;

    virtual int getFriendCircleID(const ToxPk& id) const = 0;
    virtual void setFriendCircleID(const ToxPk& id, int circleID) = 0;

    virtual QDate getFriendActivity(const ToxPk& id) const = 0;
    virtual void setFriendActivity(const ToxPk& id, const QDate& date) = 0;

    virtual void saveFriendSettings(const ToxPk& id) = 0;
    virtual void removeFriendSettings(const ToxPk& id) = 0;

signals:
    void autoAcceptCallChanged(const ToxPk& id, AutoAcceptCallFlags accept);
    void autoGroupInviteChanged(const ToxPk& id, bool accept);
};

#endif // I_FRIEND_SETTINGS_H
