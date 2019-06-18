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

#ifndef MESSAGE_H
#define MESSAGE_H

#include <QDateTime>
#include <QString>

#include <vector>

class Friend;

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
    size_t start;
    size_t end;
};

struct Message
{
    bool isAction;
    QString content;
    QDateTime timestamp;
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
        QRegExp GetNameMention()
        {
            return nameMention;
        }
        QRegExp GetSanitizedNameMention()
        {
            return sanitizedNameMention;
        }
        void onUserNameSet(const QString& username);

    private:
        QRegExp nameMention;
        QRegExp sanitizedNameMention;
    };

    MessageProcessor(SharedParams* sharedParams);

    std::vector<Message> processOutgoingMessage(bool isAction, QString const& content);

    Message processIncomingMessage(bool isAction, QString const& message);

    /**
     * @brief Enables mention detection in the processor
     */
    void enableMentions()
    {
        detectingMentions = true;
    }

    /**
     * @brief disables mention detection in the processor
     */
    void disableMentions()
    {
        detectingMentions = false;
    };

private:
    bool detectingMentions = false;
    SharedParams* sharedParams;
};

#endif /*MESSAGE_H*/