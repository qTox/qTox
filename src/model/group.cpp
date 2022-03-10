/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#include "group.h"
#include "friend.h"
#include "src/core/chatid.h"
#include "src/core/groupid.h"
#include "src/core/toxpk.h"
#include "src/friendlist.h"

#include <cassert>

#include <QDebug>

namespace {
const int MAX_GROUP_TITLE_LENGTH = 128;
} // namespace

Group::Group(int groupId_, const GroupId persistentGroupId, const QString& name, bool isAvGroupchat,
             const QString& selfName_, ICoreGroupQuery& groupQuery_, ICoreIdHandler& idHandler_,
             FriendList& friendList_)
    : groupQuery(groupQuery_)
    , idHandler(idHandler_)
    , selfName{selfName_}
    , title{name}
    , toxGroupNum(groupId_)
    , groupId{persistentGroupId}
    , avGroupchat{isAvGroupchat}
    , friendList{friendList_}
{
    // in groupchats, we only notify on messages containing your name <-- dumb
    // sound notifications should be on all messages, but system popup notification
    // on naming is appropriate
    hasNewMessages = 0;
    userWasMentioned = 0;
    regeneratePeerList();
}

void Group::setName(const QString& newTitle)
{
    const QString shortTitle = newTitle.left(MAX_GROUP_TITLE_LENGTH);
    if (!shortTitle.isEmpty() && title != shortTitle) {
        title = shortTitle;
        emit displayedNameChanged(title);
        emit titleChangedByUser(title);
        emit titleChanged(selfName, title);
    }
}

void Group::setTitle(const QString& author, const QString& newTitle)
{
    const QString shortTitle = newTitle.left(MAX_GROUP_TITLE_LENGTH);
    if (!shortTitle.isEmpty() && title != shortTitle) {
        title = shortTitle;
        emit displayedNameChanged(title);
        emit titleChanged(author, title);
    }
}

QString Group::getName() const
{
    return title;
}

QString Group::getDisplayedName() const
{
    return getName();
}

QString Group::getDisplayedName(const ToxPk& contact) const
{
    return resolveToxPk(contact);
}

void Group::regeneratePeerList()
{
    // NOTE: there's a bit of a race here. Core emits a signal for both groupPeerlistChanged and
    // groupPeerNameChanged back-to-back when a peer joins our group. If we get both before we
    // process this slot, core->getGroupPeerNames will contain the new peer name, and we'll ignore
    // the name changed signal, and emit a single userJoined with the correct name. But, if we
    // receive the name changed signal a little later, we will emit userJoined before we have their
    // username, using just their ToxPk, then shortly after emit another peerNameChanged signal.
    // This can cause double-updated to UI and chatlog, but is unavoidable given the API of toxcore.
    QStringList peers = groupQuery.getGroupPeerNames(toxGroupNum);
    const auto oldPeerNames = peerDisplayNames;
    peerDisplayNames.clear();
    const int nPeers = peers.size();
    for (int i = 0; i < nPeers; ++i) {
        const auto pk = groupQuery.getGroupPeerPk(toxGroupNum, i);
        if (pk == idHandler.getSelfPublicKey()) {
            peerDisplayNames[pk] = idHandler.getUsername();
        } else {
            peerDisplayNames[pk] = friendList.decideNickname(pk, peers[i]);
        }
    }
    for (const auto& pk : oldPeerNames.keys()) {
        if (!peerDisplayNames.contains(pk)) {
            emit userLeft(pk, oldPeerNames.value(pk));
        }
    }
    for (const auto& pk : peerDisplayNames.keys()) {
        if (!oldPeerNames.contains(pk)) {
            emit userJoined(pk, peerDisplayNames.value(pk));
        }
    }
    for (const auto& pk : peerDisplayNames.keys()) {
        if (oldPeerNames.contains(pk) && oldPeerNames.value(pk) != peerDisplayNames.value(pk)) {
            emit peerNameChanged(pk, oldPeerNames.value(pk), peerDisplayNames.value(pk));
        }
    }
    if (oldPeerNames.size() != nPeers) {
        emit numPeersChanged(nPeers);
    }
}

void Group::updateUsername(ToxPk pk, const QString newName)
{
    const QString displayName = friendList.decideNickname(pk, newName);
    assert(peerDisplayNames.contains(pk));
    if (peerDisplayNames[pk] != displayName) {
        // there could be no actual change even if their username changed due to an alias being set
        const auto oldName = peerDisplayNames[pk];
        peerDisplayNames[pk] = displayName;
        emit peerNameChanged(pk, oldName, displayName);
    }
}

bool Group::isAvGroupchat() const
{
    return avGroupchat;
}

uint32_t Group::getId() const
{
    return toxGroupNum;
}

const GroupId& Group::getPersistentId() const
{
    return groupId;
}

int Group::getPeersCount() const
{
    return peerDisplayNames.size();
}

/**
 * @brief Gets the PKs and names of all peers
 * @return PKs and names of all peers, including our own PK and name
 */
const QMap<ToxPk, QString>& Group::getPeerList() const
{
    return peerDisplayNames;
}

void Group::setEventFlag(bool f)
{
    hasNewMessages = f;
}

bool Group::getEventFlag() const
{
    return hasNewMessages;
}

void Group::setMentionedFlag(bool f)
{
    userWasMentioned = f;
}

bool Group::getMentionedFlag() const
{
    return userWasMentioned;
}

QString Group::resolveToxPk(const ToxPk& id) const
{
    auto it = peerDisplayNames.find(id);

    if (it != peerDisplayNames.end()) {
        return *it;
    }

    return QString();
}

void Group::setSelfName(const QString& name)
{
    selfName = name;
}

QString Group::getSelfName() const
{
    return selfName;
}
