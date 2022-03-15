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

#pragma once

#include "src/core/icoregroupmessagesender.h"
#include "src/core/icoreidhandler.h"
#include "src/model/group.h"
#include "src/model/imessagedispatcher.h"
#include "src/model/message.h"

#include <QObject>
#include <QString>

#include <cstdint>

class IGroupSettings;

class GroupMessageDispatcher : public IMessageDispatcher
{
    Q_OBJECT
public:
    GroupMessageDispatcher(Group& g_, MessageProcessor processor, ICoreIdHandler& idHandler,
                           ICoreGroupMessageSender& messageSender,
                           const IGroupSettings& groupSettings);

    std::pair<DispatchedMessageId, DispatchedMessageId> sendMessage(bool isAction,
                                                                    QString const& content) override;

    std::pair<DispatchedMessageId, DispatchedMessageId> sendExtendedMessage(const QString& content,
                            ExtensionSet extensions) override;
    void onMessageReceived(ToxPk const& sender, bool isAction, QString const& content);

private:
    Group& group;
    MessageProcessor processor;
    ICoreIdHandler& idHandler;
    ICoreGroupMessageSender& messageSender;
    const IGroupSettings& groupSettings;
    DispatchedMessageId nextMessageId{0};
};
