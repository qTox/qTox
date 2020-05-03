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

#pragma once

#include "contact.h"

#include "src/core/contactid.h"
#include "src/core/groupid.h"
#include "src/core/icoregroupquery.h"
#include "src/core/icoreidhandler.h"
#include "src/core/toxpk.h"

#include <QMap>
#include <QObject>
#include <QStringList>

class Group : public Contact
{
    Q_OBJECT
public:
    Group(int groupId, const GroupId persistentGroupId, const QString& name, bool isAvGroupchat,
          const QString& selfName, ICoreGroupQuery& groupQuery, ICoreIdHandler& idHandler);
    bool isAvGroupchat() const;
    uint32_t getId() const override;
    const GroupId& getPersistentId() const override;
    int getPeersCount() const;
    void regeneratePeerList();
    const QMap<ToxPk, QString>& getPeerList() const;
    bool peerHasNickname(ToxPk pk);

    void setEventFlag(bool f) override;
    bool getEventFlag() const override;

    void setMentionedFlag(bool f);
    bool getMentionedFlag() const;

    void updateUsername(ToxPk pk, const QString newName);
    void setName(const QString& newTitle) override;
    void setTitle(const QString& author, const QString& newTitle);
    QString getName() const;
    QString getDisplayedName() const override;
    QString resolveToxId(const ToxPk& id) const;
    void setSelfName(const QString& name);
    QString getSelfName() const;

    bool useHistory() const final;

signals:
    void titleChangedByUser(const QString& title);
    void titleChanged(const QString& author, const QString& title);
    void userJoined(const ToxPk& user, const QString& name);
    void userLeft(const ToxPk& user, const QString& name);
    void numPeersChanged(int numPeers);
    void peerNameChanged(const ToxPk& peer, const QString& oldName, const QString& newName);

private:
    void stopAudioOfDepartedPeers(const ToxPk& peerPk);

private:
    ICoreGroupQuery& groupQuery;
    ICoreIdHandler& idHandler;
    QString selfName;
    QString title;
    QMap<ToxPk, QString> peerDisplayNames;
    bool hasNewMessages;
    bool userWasMentioned;
    int toxGroupNum;
    const GroupId groupId;
    bool avGroupchat;
};
