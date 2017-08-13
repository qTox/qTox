#include "usermessage.h"

/**
 * @class UserMessage
 * @brief Message which has an author (sender)
 *
 * @var author Author of a message
 */
UserMessage::UserMessage(const ToxPk& author, const QDateTime& time)
    : Message{time}
    , author{author}
{
}

void UserMessage::setAuthor(const ToxPk& author)
{
    this->author = author;
}

const ToxPk& UserMessage::getAuthor() const
{
    return author;
}
