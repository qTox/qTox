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

#include "message.h"
#include "friend.h"
#include "src/core/core.h"

#include <cassert>

void MessageProcessor::SharedParams::onUserNameSet(const QString& username)
{
    QString sanename = username;
    sanename.remove(QRegularExpression("[\\t\\n\\v\\f\\r\\x0000]"));
    nameMention = QRegularExpression("\\b" + QRegularExpression::escape(username) + "\\b",
                                     QRegularExpression::CaseInsensitiveOption);
    sanitizedNameMention = QRegularExpression("\\b" + QRegularExpression::escape(sanename) + "\\b",
                                              QRegularExpression::CaseInsensitiveOption);
}

/**
 * @brief Set the public key on which a message should be highlighted
 * @param pk ToxPk in its hex string form
 */
void MessageProcessor::SharedParams::setPublicKey(const QString& pk)
{
    // no sanitization needed, we expect a ToxPk in its string form
    pubKeyMention = QRegularExpression("\\b" + pk + "\\b",
                                       QRegularExpression::CaseInsensitiveOption);
}

MessageProcessor::MessageProcessor(const MessageProcessor::SharedParams& sharedParams)
    : sharedParams(sharedParams)
{}

/**
 * @brief Converts an outgoing message into one (or many) sanitized Message(s)
 */
std::vector<Message> MessageProcessor::processOutgoingMessage(bool isAction, QString const& content, ExtensionSet extensions)
{
    std::vector<Message> ret;

    const auto needsSplit = !extensions[ExtensionType::messages] || isAction;
    const auto splitMsgs = needsSplit
        ? Core::splitMessage(content)
        : QStringList({content});

    ret.reserve(splitMsgs.size());

    QDateTime timestamp = QDateTime::currentDateTime();
    std::transform(splitMsgs.begin(), splitMsgs.end(), std::back_inserter(ret),
                   [&](const QString& part) {
                       Message message;
                       message.isAction = isAction;
                       message.content = part;
                       message.timestamp = timestamp;
                       // In theory we could limit this only to the extensions
                       // required but since Core owns the splitting logic it
                       // isn't trivial to do that now
                       message.extensionSet = extensions;
                       return message;
                   });

    return ret;
}

/**
 * @brief Converts an incoming message into a sanitized Message
 */
Message MessageProcessor::processIncomingCoreMessage(bool isAction, QString const& message)
{
    QDateTime timestamp = QDateTime::currentDateTime();
    auto ret = Message{};
    ret.isAction = isAction;
    ret.content = message;
    ret.timestamp = timestamp;

    if (detectingMentions) {
        auto nameMention = sharedParams.GetNameMention();
        auto sanitizedNameMention = sharedParams.GetSanitizedNameMention();
        auto pubKeyMention = sharedParams.GetPublicKeyMention();

        for (auto const& mention : {nameMention, sanitizedNameMention, pubKeyMention}) {
            auto matchIt = mention.globalMatch(ret.content);
            if (!matchIt.hasNext()) {
                continue;
            }

            auto match = matchIt.next();

            auto pos = static_cast<size_t>(match.capturedStart());
            auto length = static_cast<size_t>(match.capturedLength());

            // skip matches on empty usernames
            if (length == 0) {
                continue;
            }

            ret.metadata.push_back({MessageMetadataType::selfMention, pos, pos + length});
            break;
        }
    }

    return ret;
}

Message MessageProcessor::processIncomingExtMessage(const QString& content)
{
    // Note: detectingMentions not implemented here since mentions are only
    // currently useful in group messages which do not support extensions. If we
    // were to support mentions we would probably want to do something more
    // intelligent anyways
    assert(detectingMentions == false);
    auto message = Message();
    message.timestamp = QDateTime::currentDateTime();
    message.content = content;
    message.extensionSet |= ExtensionType::messages;

    return message;
}
