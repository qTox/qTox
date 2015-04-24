/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#ifndef CHATMESSAGE_H
#define CHATMESSAGE_H

#include "chatline.h"
#include "src/core/corestructs.h"
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

    static ChatMessage::Ptr createChatMessage(const QString& sender, const QString& rawMessage, MessageType type, bool isMe, const QDateTime& date = QDateTime());
    static ChatMessage::Ptr createChatInfoMessage(const QString& rawMessage, SystemMessageType type, const QDateTime& date);
    static ChatMessage::Ptr createFileTransferMessage(const QString& sender, ToxFile file, bool isMe, const QDateTime& date);
    static ChatMessage::Ptr createTypingNotification();
    static ChatMessage::Ptr createBusyNotification();

    void markAsSent(const QDateTime& time);
    QString toString() const;
    bool isAction() const;
    void setAsAction();
    void hideSender();
    void hideDate();

protected:
    static QString detectAnchors(const QString& str);
    static QString detectQuotes(const QString& str);
    static QString wrapDiv(const QString& str, const QString& div);

private:
    bool action = false;
};

#endif // CHATMESSAGE_H
