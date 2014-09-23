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

#include "messageaction.h"
#include "smileypack.h"

MessageAction::MessageAction(const QString &author, const QString &message, const QString &date, const bool &me) :
    ChatAction(me, author, date),
    message(message)
{
}

void MessageAction::setTextCursor(QTextCursor cursor)
{
    // When this function is called, we're supposed to only update ourselve when needed
    // Nobody should ask us to do anything with our content, we're on our own
    // Except we never udpate on our own, so we can safely free our resources

    (void) cursor;
    message.clear();
    message.squeeze();
    name.clear();
    name.squeeze();
    date.clear();
    date.squeeze();
}

QString MessageAction::getMessage()
{
    QString message_ = SmileyPack::getInstance().smileyfied(toHtmlChars(message));

    // detect urls
    QRegExp exp("(www\\.|http[s]?:\\/\\/|ftp:\\/\\/)\\S+");
    int offset = 0;
    while ((offset = exp.indexIn(message_, offset)) != -1)
    {
        QString url = exp.cap();

        // add scheme if not specified
        if (exp.cap(1) == "www.")
            url.prepend("http://");

        QString htmledUrl = QString("<a href=\"%1\">%1</a>").arg(url);
        message_.replace(offset, exp.cap().length(), htmledUrl);

        offset += htmledUrl.length();
    }

    // detect text quotes
    QStringList messageLines = message_.split("\n");
    message_ = "";
    for (QString& s : messageLines)
    {
        if (QRegExp("^[ ]*&gt;.*").exactMatch(s))
            message_ += "<span class=quote>>" + s.right(s.length()-4) + "</span><br/>";
        else
            message_ += s + "<br/>";
    }
    message_ = message_.left(message_.length()-4);

    return QString("<div class=message>" + message_ + "</div>");
}
