#include "textmessage.h"

static const QString ACTION_PREFIX{QStringLiteral("/me ")};

TextMessage::TextMessage(int id, const ToxPk& author, const QString& text, const QDateTime& time)
    : UserMessage{author, time}
    , id{id}
{
    _isAction = text.startsWith(ACTION_PREFIX);
    this->text = _isAction ? text.mid(ACTION_PREFIX.length()) : text;
}

TextMessage::TextMessage(int id, const ToxPk& author, const QString& text, const QDateTime& time, bool isAction)
    : UserMessage{author, time}
    , id{id}
    , text{text}
    , _isAction{isAction}
{
}

TextMessage::TextMessage(const TextMessage& message)
    : UserMessage{message.getAuthor(), message.getTime()}
    , id{id}
    , text{message.text}
    , _isAction{message._isAction}
{
}

TextMessage& TextMessage::operator=(const TextMessage& message)
{
    id = message.id;
    setAuthor(message.getAuthor());
    text = message.text;
    setTime(message.getTime());
    _isAction = message._isAction;
}

TextMessage::~TextMessage()
{
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
    return _isAction;
}

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
