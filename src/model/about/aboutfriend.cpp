#include "aboutfriend.h"

#include "src/model/friend.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"

AboutFriend::AboutFriend(const Friend* f)
    : f{f}
{
    Settings* s = &Settings::getInstance();
    connect(s, &Settings::contactNoteChanged, [=](const ToxPk& pk, const QString& note) {
        emit noteChanged(note);
    });
    connect(s, &Settings::autoAcceptCallChanged,
            [=](const ToxPk& pk, IFriendSettings::AutoAcceptCallFlags flag) {
        emit autoAcceptCallChanged(flag);
    });
    connect(s, &Settings::autoAcceptDirChanged, [=](const ToxPk& pk, const QString& dir) {
        emit autoAcceptDirChanged(dir);
    });
    connect(s, &Settings::autoGroupInviteChanged, [=](const ToxPk& pk, bool enable) {
        emit autoGroupInviteChanged(enable);
    });
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
    const ToxPk pk = f->getPublicKey();
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
}

IFriendSettings::AutoAcceptCallFlags AboutFriend::getAutoAcceptCall() const
{
    const ToxPk pk = f->getPublicKey();
    return Settings::getInstance().getAutoAcceptCall(pk);
}

void AboutFriend::setAutoAcceptCall(IFriendSettings::AutoAcceptCallFlags flag)
{
    const ToxPk pk = f->getPublicKey();
    Settings::getInstance().setAutoAcceptCall(pk, flag);
    Settings::getInstance().savePersonal();
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
}

bool AboutFriend::clearHistory()
{
    const ToxPk pk = f->getPublicKey();
    History* const history = Nexus::getProfile()->getHistory();
    if (history) {
        history->removeFriendHistory(pk.toString());
        return true;
    }

    return false;
}
