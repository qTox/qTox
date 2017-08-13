#include "usermessage.h"

UserMessage::UserMessage(const ToxPk& author, const QDateTime& time)
    : Message{time}
    , author{author}
{
}

UserMessage::~UserMessage()
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
