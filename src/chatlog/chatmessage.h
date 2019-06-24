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

#ifndef CHATMESSAGE_H
#define CHATMESSAGE_H

#include "chatline.h"
#include "src/core/toxfile.h"
#include <QDateTime>

class QGraphicsScene;

class ChatMessage : public ChatLine
{
public:
    using Ptr = std::shared_ptr<ChatMessage>;

    enum SystemMessageType
    {
        INFO,
        ERROR,
        TYPING,
    };

    enum MessageType
    {
        NORMAL,
        ACTION,
        ALERT,
    };

    ChatMessage();

    static ChatMessage::Ptr createChatMessage(const QString& sender, const QString& rawMessage,
                                              MessageType type, bool isMe,
                                              const QDateTime& date = QDateTime(), bool colorizeName = false);
    static ChatMessage::Ptr createChatInfoMessage(const QString& rawMessage, SystemMessageType type,
                                                  const QDateTime& date);
    static ChatMessage::Ptr createFileTransferMessage(const QString& sender, ToxFile file,
                                                      bool isMe, const QDateTime& date);
    static ChatMessage::Ptr createTypingNotification();
    static ChatMessage::Ptr createBusyNotification();

    void markAsSent(const QDateTime& time);
    QString toString() const;
    bool isAction() const;
    void setAsAction();
    void hideSender();
    void hideDate();

protected:
    static QString detectQuotes(const QString& str, MessageType type);
    static QString wrapDiv(const QString& str, const QString& div);

private:
    bool action = false;
};

#endif // CHATMESSAGE_H
