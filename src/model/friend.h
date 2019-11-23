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

#ifndef FRIEND_H
#define FRIEND_H

#include "contact.h"
#include "src/core/core.h"
#include "src/core/extension.h"
#include "src/core/toxid.h"
#include "src/core/contactid.h"
#include "src/model/status.h"
#include <QObject>
#include <QString>

class Friend : public Contact
{
    Q_OBJECT
public:
    Friend(uint32_t friendId, const ToxPk& friendPk, const QString& userAlias = {}, const QString &userName = {});
    Friend(const Friend& other) = delete;
    Friend& operator=(const Friend& other) = delete;

    void setName(const QString& name) override;
    void setAlias(const QString& name);
    QString getDisplayedName() const override;
    bool hasAlias() const;
    QString getUserName() const;
    void setStatusMessage(const QString& message);
    QString getStatusMessage() const;

    void setEventFlag(bool f) override;
    bool getEventFlag() const override;

    const ToxPk& getPublicKey() const;
    uint32_t getId() const override;
    const ContactId& getPersistentId() const override;

    void finishNegotiation();
    void setStatus(Status::Status s);
    Status::Status getStatus() const;
    bool isOnline() const;

    bool useHistory() const final;

    void setExtendedMessageSupport(bool supported);
    ExtensionSet getSupportedExtensions() const;

signals:
    void nameChanged(const ToxPk& friendId, const QString& name);
    void aliasChanged(const ToxPk& friendId, QString alias);
    void statusChanged(const ToxPk& friendId, Status::Status status);
    void statusMessageChanged(const ToxPk& friendId, const QString& message);
    void extensionSupportChanged(ExtensionSet extensions);
    void loadChatHistory();

public slots:
private:
    QString userName;
    QString userAlias;
    QString statusMessage;
    ToxPk friendPk;
    uint32_t friendId;
    bool hasNewEvents;
    Status::Status friendStatus;
    ExtensionSet supportedExtensions;
};

#endif // FRIEND_H
