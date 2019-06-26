/*
    Copyright © 2019 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "aboutfriend.h"

#include "src/model/friend.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/ifriendsettings.h"

AboutFriend::AboutFriend(const Friend* f, IFriendSettings* const s)
    : f{f}
    , settings{s}
{
    s->connectTo_contactNoteChanged([=](const ToxPk& pk, const QString& note) {
        emit noteChanged(note);
    });
    s->connectTo_autoAcceptCallChanged(
            [=](const ToxPk& pk, IFriendSettings::AutoAcceptCallFlags flag) {
        emit autoAcceptCallChanged(flag);
    });
    s->connectTo_autoAcceptDirChanged([=](const ToxPk& pk, const QString& dir) {
        emit autoAcceptDirChanged(dir);
    });
    s->connectTo_autoGroupInviteChanged([=](const ToxPk& pk, bool enable) {
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

ToxPk AboutFriend::getPublicKey() const
{
    return f->getPublicKey();
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
}

IFriendSettings::AutoAcceptCallFlags AboutFriend::getAutoAcceptCall() const
{
    const ToxPk pk = f->getPublicKey();
    return settings->getAutoAcceptCall(pk);
}

void AboutFriend::setAutoAcceptCall(IFriendSettings::AutoAcceptCallFlags flag)
{
    const ToxPk pk = f->getPublicKey();
    settings->setAutoAcceptCall(pk, flag);
    settings->saveFriendSettings(pk);
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

bool AboutFriend::isHistoryExistence()
{
    History* const history = Nexus::getProfile()->getHistory();
    if (history) {
        const ToxPk pk = f->getPublicKey();
        return history->historyExists(pk);
    }

    return false;
}
