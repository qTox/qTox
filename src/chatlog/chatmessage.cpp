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

#include "chatmessage.h"
#include "chatlinecontentproxy.h"
#include "content/text.h"
#include "content/spinner.h"
#include "content/filetransferwidget.h"
#include "content/image.h"

#include "src/misc/settings.h"
#include "src/misc/smileypack.h"
#include "src/misc/style.h"

#define NAME_COL_WIDTH 90.0
#define TIME_COL_WIDTH 90.0

ChatMessage::ChatMessage()
{

}

ChatMessage::Ptr ChatMessage::createChatMessage(const QString &sender, const QString &rawMessage, bool isAction, bool alert, bool isMe, const QDateTime &date)
{
    ChatMessage::Ptr msg = ChatMessage::Ptr(new ChatMessage);

    QString text = toHtmlChars(rawMessage);

    //smileys
    if(Settings::getInstance().getUseEmoticons())
        text = SmileyPack::getInstance().smileyfied(text);

    //quotes (green text)
    text = detectQuotes(detectAnchors(text));

    if(isAction)
    {
        text = QString("<div class=action>%1 %2</div>").arg(sender, text);
        msg->setAsAction();
    }
    else if(alert)
    {
        text = "<div class=alert>" + text + "</div>";
    }

    msg->addColumn(new Text(isAction ? "<div class=action>*</div>" : sender, isMe ? Style::getFont(Style::BigBold) : Style::getFont(Style::Big), isAction ? false : true), ColumnFormat(NAME_COL_WIDTH, ColumnFormat::FixedSize, ColumnFormat::Right));
    msg->addColumn(new Text(text, Style::getFont(Style::Big), false, rawMessage), ColumnFormat(1.0, ColumnFormat::VariableSize));
    msg->addColumn(new Spinner(QSizeF(16, 16)), ColumnFormat(TIME_COL_WIDTH, ColumnFormat::FixedSize, ColumnFormat::Right));

    if(!date.isNull())
        msg->markAsSent(date);

    return msg;
}

ChatMessage::Ptr ChatMessage::createChatInfoMessage(const QString &rawMessage, SystemMessageType type, const QDateTime &date)
{
    ChatMessage::Ptr msg = ChatMessage::Ptr(new ChatMessage);

    QString img;
    switch(type)
    {
    case INFO:   img = ":/ui/chatArea/info.png";     break;
    case ERROR:  img = ":/ui/chatArea/error.png";    break;
    case TYPING: img = ":/ui/chatArea/typing.png";   break;
    }

    msg->addColumn(new Image(QSizeF(16, 16), img), ColumnFormat(NAME_COL_WIDTH, ColumnFormat::FixedSize, ColumnFormat::Right));
    msg->addColumn(new Text(rawMessage, Style::getFont(Style::Big), false, rawMessage), ColumnFormat(1.0, ColumnFormat::VariableSize, ColumnFormat::Center));
    msg->addColumn(new Text(date.toString(Settings::getInstance().getTimestampFormat()), Style::getFont(Style::Big)), ColumnFormat(TIME_COL_WIDTH, ColumnFormat::FixedSize, ColumnFormat::Right));

    return msg;
}

ChatMessage::Ptr ChatMessage::createFileTransferMessage(const QString& sender, ToxFile file, bool isMe, const QDateTime& date)
{
    ChatMessage::Ptr msg = ChatMessage::Ptr(new ChatMessage);

    msg->addColumn(new Text(sender, isMe ? Style::getFont(Style::BigBold) : Style::getFont(Style::Big), true), ColumnFormat(NAME_COL_WIDTH, ColumnFormat::FixedSize, ColumnFormat::Right));
    msg->addColumn(new ChatLineContentProxy(new FileTransferWidget(0, file), 350, 0.6f), ColumnFormat(1.0, ColumnFormat::VariableSize));
    msg->addColumn(new Text(date.toString(Settings::getInstance().getTimestampFormat()), Style::getFont(Style::Big)), ColumnFormat(TIME_COL_WIDTH, ColumnFormat::FixedSize, ColumnFormat::Right));

    return msg;
}

void ChatMessage::markAsSent(const QDateTime &time)
{
    // remove the spinner and replace it by $time
    replaceContent(2, new Text(time.toString(Settings::getInstance().getTimestampFormat())));
}

QString ChatMessage::toString() const
{
    ChatLineContent* c = getContent(1);
    if(c)
        return c->getText();

    return QString();
}

bool ChatMessage::isAction() const
{
    return action;
}

void ChatMessage::setAsAction()
{
    action = true;
}

void ChatMessage::hideSender()
{
    ChatLineContent* c = getContent(0);
    if(c)
        c->hide();
}

void ChatMessage::hideDate()
{
    ChatLineContent* c = getContent(2);
    if(c)
        c->hide();
}

QString ChatMessage::detectAnchors(const QString &str)
{
    QString out = str;

    // detect urls
    QRegExp exp("(?:\\b)(www\\.|http[s]?:\\/\\/|ftp:\\/\\/|tox:\\/\\/|tox:)\\S+");
    int offset = 0;
    while ((offset = exp.indexIn(out, offset)) != -1)
    {
        QString url = exp.cap();

        // If there's a trailing " it's a HTML attribute, e.g. a smiley img's title=":tox:"
        if (url == "tox:\"")
        {
            offset += url.length();
            continue;
        }

        // add scheme if not specified
        if (exp.cap(1) == "www.")
            url.prepend("http://");

        QString htmledUrl = QString("<a href=\"%1\">%1</a>").arg(url);
        out.replace(offset, exp.cap().length(), htmledUrl);

        offset += htmledUrl.length();
    }

    return out;
}

QString ChatMessage::detectQuotes(const QString& str)
{
    // detect text quotes
    QStringList messageLines = str.split("\n");
    QString quotedText;
    for (int i=0;i<messageLines.size();++i)
    {
        if (QRegExp("^[ ]*&gt;.*").exactMatch(messageLines[i]))
            quotedText += "<span class=quote>" + messageLines[i] + "</span>";
        else
            quotedText += messageLines[i];

        if (i < messageLines.size() - 1)
            quotedText += "<br/>";
    }

    return quotedText;
}

QString ChatMessage::toHtmlChars(const QString &str)
{
    static QList<QPair<QString, QString>> replaceList = {{"&","&amp;"}, {">","&gt;"}, {"<","&lt;"}};
    QString res = str;

    for (auto &it : replaceList)
        res = res.replace(it.first,it.second);

    return res;
}
