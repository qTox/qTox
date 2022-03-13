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

#include "chatline.h"
#include "src/core/toxfile.h"
#include "src/persistence/history.h"

#include <QDateTime>

class CoreFile;
class QGraphicsScene;
class DocumentCache;
class SmileyPack;
class Settings;
class Style;
class IMessageBoxManager;

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

    ChatMessage(DocumentCache& documentCache, Settings& settings, Style& style);
    ~ChatMessage();
    ChatMessage(const ChatMessage&) = default;
    ChatMessage(ChatMessage&&) = default;

    static ChatMessage::Ptr createChatMessage(const QString& sender, const QString& rawMessage,
                                              MessageType type, bool isMe, MessageState state,
                                              const QDateTime& date, DocumentCache& documentCache,
                                              SmileyPack& smileyPack, Settings& settings, Style& style, bool colorizeName = false);
    static ChatMessage::Ptr createChatInfoMessage(const QString& rawMessage, SystemMessageType type,
                                                  const QDateTime& date, DocumentCache& documentCache, Settings& settings,
                                                  Style& style);
    static ChatMessage::Ptr createFileTransferMessage(const QString& sender, CoreFile& coreFile,
                                                      ToxFile file, bool isMe, const QDateTime& date,
                                                      DocumentCache& documentCache, Settings& settings, Style& style, IMessageBoxManager& messageBoxManager);
    static ChatMessage::Ptr createTypingNotification(DocumentCache& documentCache, Settings& settings, Style& style);
    static ChatMessage::Ptr createBusyNotification(DocumentCache& documentCache, Settings& settings, Style& style);

    void markAsDelivered(const QDateTime& time);
    void markAsBroken();
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
    DocumentCache& documentCache;
    Settings& settings;
    Style& style;
};
