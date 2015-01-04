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
#include "src/corestructs.h"
#include <QDateTime>

class QGraphicsScene;

class ChatMessage : public ChatLine
{
public:
    using Ptr = std::shared_ptr<ChatMessage>;

    ChatMessage(QGraphicsScene* scene, const QString& rawMessage);

    static ChatMessage::Ptr createChatMessage(QGraphicsScene* scene, const QString& sender, const QString& rawMessage, bool isAction, bool alert, bool isMe, const QDateTime& date = QDateTime());
    static ChatMessage::Ptr createChatInfoMessage(QGraphicsScene* scene, const QString& rawMessage, const QString& type, const QDateTime& date);
    static ChatMessage::Ptr createFileTransferMessage(QGraphicsScene* scene, const QString& sender, const QString& rawMessage, ToxFile file, bool isMe, const QDateTime& date);

    void markAsSent(const QDateTime& time);
    QString toString() const;
    bool isAction() const;
    void setAsAction();

protected:
    static QString detectAnchors(const QString& str);
    static QString detectQuotes(const QString& str);
    static QString toHtmlChars(const QString& str);

private:
    ChatLineContent* midColumn = nullptr;
    QString rawString;
    bool action = false;
};

#endif // CHATMESSAGE_H
