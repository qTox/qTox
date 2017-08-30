#ifndef ABOUT_FRIEND_H
#define ABOUT_FRIEND_H

#include "iaboutfriend.h"

#include <QObject>

class Friend;
class IFriendSettings;

class AboutFriend : public IAboutFriend
{
    Q_OBJECT

public:
    explicit AboutFriend(const Friend* f, IFriendSettings* settings);

    QString getName() const override;
    QString getStatusMessage() const override;
    QString getPublicKey() const override;

    QPixmap getAvatar() const override;

    QString getNote() const override;
    void setNote(const QString& note) override;

    QString getAutoAcceptDir() const override;
    void setAutoAcceptDir(const QString& path) override;

    AutoAcceptCall getAutoAcceptCall() const override;
    void setAutoAcceptCall(AutoAcceptCall flag) override;

    bool getAutoGroupInvite() const override;
    void setAutoGroupInvite(bool enabled) override;

    bool clearHistory() override;

private:
    const Friend* f;
    IFriendSettings* const settings;
};

#endif // ABOUT_FRIEND_H
