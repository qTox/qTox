#include "aboutfriend.h"

#include "src/model/friend.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/ifriendsettings.h"

AboutFriend::AboutFriend(const Friend* f, IFriendSettings* settings)
    : f{f}
    , settings{settings}
{
}

QString AboutFriend::getName() const
{
    return f->getDisplayedName();
}

QString AboutFriend::getStatusMessage() const
{
    return f->getStatusMessage();
}

QString AboutFriend::getPublicKey() const
{
    return f->getPublicKey().toString();
}

QPixmap AboutFriend::getAvatar() const
{
    const QString pk = f->getPublicKey().toString();
    const QPixmap avatar = Nexus::getProfile()->loadAvatar(pk);
    return avatar.isNull() ? QPixmap(":/img/contact_dark.svg")
                           : avatar;
}

QString AboutFriend::getNote() const
{
    const ToxPk pk = f->getPublicKey();
    return settings->getContactNote(pk);
}

void AboutFriend::setNote(const QString& note)
{
    const ToxPk pk = f->getPublicKey();
    settings->setContactNote(pk, note);
    settings->saveFriendSettings(pk);
}

QString AboutFriend::getAutoAcceptDir() const
{
    const ToxPk pk = f->getPublicKey();
    return settings->getAutoAcceptDir(pk);
}

void AboutFriend::setAutoAcceptDir(const QString& path)
{
    const ToxPk pk = f->getPublicKey();
    settings->setAutoAcceptDir(pk, path);
    settings->saveFriendSettings(pk);
    emit autoAcceptDirChanged(path);
}

IAboutFriend::AutoAcceptCall AboutFriend::getAutoAcceptCall() const
{
    const ToxPk pk = f->getPublicKey();
    const int value = (int)settings->getAutoAcceptCall(pk);
    return (IAboutFriend::AutoAcceptCall)value;
}

void AboutFriend::setAutoAcceptCall(IAboutFriend::AutoAcceptCall flag)
{
    const ToxPk pk = f->getPublicKey();
    const int value = (int)flag;
    const IFriendSettings::AutoAcceptCallFlags sFlag(value);
    settings->setAutoAcceptCall(pk, sFlag);
    settings->saveFriendSettings(pk);
    emit autoAcceptCallChanged(flag);
}

bool AboutFriend::getAutoGroupInvite() const
{
    const ToxPk pk = f->getPublicKey();
    return settings->getAutoGroupInvite(pk);
}

void AboutFriend::setAutoGroupInvite(bool enabled)
{
    const ToxPk pk = f->getPublicKey();
    settings->setAutoGroupInvite(pk, enabled);
    settings->saveFriendSettings(pk);
    emit autoGroupInviteChaged(enabled);
}

bool AboutFriend::clearHistory()
{
    const ToxPk pk = f->getPublicKey();
    History* history = Nexus::getProfile()->getHistory();
    if (history) {
        history->removeFriendHistory(pk.toString());
    }
}
