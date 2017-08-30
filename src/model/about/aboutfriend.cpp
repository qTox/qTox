#include "aboutfriend.h"

#include "src/model/friend.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"

AboutFriend::AboutFriend(const Friend* f)
    : f{f}
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
    return avatar.isNull() ? QPixmap(QStringLiteral(":/img/contact_dark.svg"))
                           : avatar;
}

QString AboutFriend::getNote() const
{
    const ToxPk pk = f->getPublicKey();
    return Settings::getInstance().getContactNote(pk);
}

void AboutFriend::setNote(const QString& note)
{
    const ToxPk pk = f->getPublicKey();
    Settings::getInstance().setContactNote(pk, note);
    Settings::getInstance().savePersonal();
}

QString AboutFriend::getAutoAcceptDir() const
{
    const ToxPk pk = f->getPublicKey();
    return Settings::getInstance().getAutoAcceptDir(pk);
}

void AboutFriend::setAutoAcceptDir(const QString& path)
{
    const ToxPk pk = f->getPublicKey();
    Settings::getInstance().setAutoAcceptDir(pk, path);
    Settings::getInstance().savePersonal();
    emit autoAcceptDirChanged(path);
}

IAboutFriend::AutoAcceptCall AboutFriend::getAutoAcceptCall() const
{
    const ToxPk pk = f->getPublicKey();
    const int value = static_cast<int>(Settings::getInstance().getAutoAcceptCall(pk));
    return (IAboutFriend::AutoAcceptCall)value;
}

void AboutFriend::setAutoAcceptCall(IAboutFriend::AutoAcceptCall flag)
{
    const ToxPk pk = f->getPublicKey();
    const int value = static_cast<int>(flag);
    const Settings::AutoAcceptCallFlags sFlag(value);
    Settings::getInstance().setAutoAcceptCall(pk, sFlag);
    Settings::getInstance().savePersonal();
    emit autoAcceptCallChanged(flag);
}

bool AboutFriend::getAutoGroupInvite() const
{
    const ToxPk pk = f->getPublicKey();
    return Settings::getInstance().getAutoGroupInvite(pk);
}

void AboutFriend::setAutoGroupInvite(bool enabled)
{
    const ToxPk pk = f->getPublicKey();
    Settings::getInstance().setAutoGroupInvite(pk, enabled);
    Settings::getInstance().savePersonal();
    emit autoGroupInviteChaged(enabled);
}

bool AboutFriend::clearHistory()
{
    const ToxPk pk = f->getPublicKey();
    History* const history = Nexus::getProfile()->getHistory();
    if (history) {
        history->removeFriendHistory(pk.toString());
    }
}
