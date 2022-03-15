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

#include "src/core/coreext.h"
#include "src/core/extension.h"

#include <QDateTime>
#include <QRegularExpression>
#include <QString>

#include <vector>

class Friend;
class CoreExt;

// NOTE: This could be extended in the future to handle all text processing (see
// ChatMessage::createChatMessage)
enum class MessageMetadataType
{
    selfMention,
};

// May need to be extended in the future to have a more varianty type (imagine
// if we wanted to add message replies and shoved a reply id in here)
struct MessageMetadata
{
    MessageMetadataType type;
    // Indicates start position within a Message::content
    size_t start;
    // Indicates end position within a Message::content
    size_t end;
};

struct Message
{
    bool isAction;
    QString content;
    QDateTime timestamp;
    ExtensionSet extensionSet;
    std::vector<MessageMetadata> metadata;
};


class MessageProcessor
{

public:
    /**
     * Parameters needed by all message processors. Used to reduce duplication
     * of expensive data looked at by all message processors
     */
    class SharedParams
    {

    public:
        SharedParams(uint64_t maxCoreMessageSize_, uint64_t maxExtendedMessageSize_)
            : maxCoreMessageSize(maxCoreMessageSize_)
            , maxExtendedMessageSize(maxExtendedMessageSize_)
        {}

        QRegularExpression getNameMention() const
        {
            return nameMention;
        }
        QRegularExpression getSanitizedNameMention() const
        {
            return sanitizedNameMention;
        }
        QRegularExpression getPublicKeyMention() const
        {
            return pubKeyMention;
        }
        void onUserNameSet(const QString& username);
        void setPublicKey(const QString& pk);

        uint64_t getMaxCoreMessageSize() const
        {
            return maxCoreMessageSize;
        }

        uint64_t getMaxExtendedMessageSize() const
        {
            return maxExtendedMessageSize;
        }

    private:
        uint64_t maxCoreMessageSize;
        uint64_t maxExtendedMessageSize;
        QRegularExpression nameMention;
        QRegularExpression sanitizedNameMention;
        QRegularExpression pubKeyMention;
    };

    MessageProcessor(const SharedParams& sharedParams_);

    std::vector<Message> processOutgoingMessage(bool isAction, const QString& content, ExtensionSet extensions);
    Message processIncomingCoreMessage(bool isAction, const QString& message);
    Message processIncomingExtMessage(const QString& content);

    /**
     * @brief Enables mention detection in the processor
     */
    inline void enableMentions()
    {
        detectingMentions = true;
    }

    /**
     * @brief Disables mention detection in the processor
     */
    inline void disableMentions()
    {
        detectingMentions = false;
    }

private:
    bool detectingMentions = false;
    const SharedParams& sharedParams;
};
