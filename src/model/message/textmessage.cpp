#include "textmessage.h"

/**
 * @class TextMessage
 * @brief Class provide all needed data to work with simple text message.
 *
 * Definition: "Full-text form" -- The format in which the user types message
 * and it stores in the database. For regular text messages, it is the text
 * itself, but for action message it is "/me " + text.
 *
 * This class stores information whether the message is an action. It can parse
 * full-text form and provide it.
 *
 * @todo Remove database id from TextMessage.
 * @var id ID of the message in the database.
 * @var text Text of a message.
 * @var _isAction Flag that indicates that a message is an action.
 */

/**
 * @var ACTION_PREFIX
 * @brief Prefix which must be appended to text of action message to represet
 * it in full-text form.
 */
static const QString ACTION_PREFIX{QStringLiteral("/me ")};

/**
 * @brief TextMessage constructor overload, which parses the full-text form.
 *
 * @param id ID of the message in the database (0 if not stored yet).
 * @param author An author of a message.
 * @param text Full-text form of a message.
 * @param time The time when the message was sent.
 */
TextMessage::TextMessage(int id, const ToxPk& author, const QString& text, const QDateTime& time)
    : UserMessage{author, time}
    , id{id}
    , mIsAction{text.startsWith(ACTION_PREFIX)}
{
    this->text = mIsAction ? text.mid(ACTION_PREFIX.length()) : text;
}

/**
 * @brief TextMessage constructor overload, that get text and a flag that
 * indicates whether this text should be interpreted as an action message
 *
 * @param id ID of the message in the database (0 if not stored yet).
 * @param author An author of a message.
 * @param text Plain message text
 * @param time The time when the message was sent.
 * @param isAction Flag that indicates whether this text should be interpreted
 * as an action message
 */
TextMessage::TextMessage(int id, const ToxPk& author, const QString& text, const QDateTime& time, bool isAction)
    : UserMessage{author, time}
    , id{id}
    , text{text}
    , mIsAction{isAction}
{
}

TextMessage::TextMessage(const TextMessage& message)
    : UserMessage{message.getAuthor(), message.getTime()}
    , id{id}
    , text{message.text}
    , mIsAction{message.mIsAction}
{
}

TextMessage& TextMessage::operator=(const TextMessage& message)
{
    id = message.id;
    setAuthor(message.getAuthor());
    text = message.text;
    setTime(message.getTime());
    mIsAction = message.mIsAction;
}

void TextMessage::setText(const QString& text)
{
    this->text = text;
}

const QString& TextMessage::getText() const
{
    return text;
}

bool TextMessage::isAction() const
{
    return mIsAction;
}

/**
 * @brief Get full-text form of an action message
 * @param text Plain text message
 * @return Message in full-text form
 */
QString TextMessage::makeActionText(const QString& text)
{
    return ACTION_PREFIX + text;
}

void TextMessage::setId(int id)
{
    this->id = id;
}

int TextMessage::getId() const
{
    return id;
}
