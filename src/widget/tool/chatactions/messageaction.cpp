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
#include "src/misc/smileypack.h"
#include "src/misc/settings.h"
#include <QTextTable>

MessageAction::MessageAction(const QString &author, const QString &message, const QString &date, const bool &me) :
    ChatAction(me, author, date),
    message(message)
{
    isProcessed = false;
}

QString MessageAction::getMessage(QString div)
{
    QString message_;
    if (Settings::getInstance().getUseEmoticons())
         message_ = SmileyPack::getInstance().smileyfied(toHtmlChars(message));
    else
         message_ = toHtmlChars(message);

    // detect urls
    QRegExp exp("(?:\\b)(www\\.|http[s]?:\\/\\/|ftp:\\/\\/|tox:\\/\\/|tox:)\\S+");
    int offset = 0;
    while ((offset = exp.indexIn(message_, offset)) != -1)
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
        message_.replace(offset, exp.cap().length(), htmledUrl);

        offset += htmledUrl.length();
    }

    // detect text quotes
    QStringList messageLines = message_.split("\n");
    message_ = "";
    for (QString& s : messageLines)
    {
        if (QRegExp("^&gt; ?\\w.*").exactMatch(s))
            message_ += "<span class=quote>" + s + "</span><br/>";
        else
            message_ += s + "<br/>";
    }
    message_ = message_.left(message_.length()-4);

    return QString(QString("<div class=%1>").arg(div) + message_ + "</div>");
}

QString MessageAction::getMessage()
{
    if (isMe)
        return getMessage("message_me");
    else
        return getMessage("message");
}

void MessageAction::featureUpdate()
{
    QTextTableCell cell = textTable->cellAt(0,3);
    QTextTableCellFormat format;
    if (!isProcessed)
        format.setBackground(QColor(Qt::red));
    else
        format.setBackground(QColor(Qt::white));
    cell.setFormat(format);
}

void MessageAction::markAsSent()
{
    isProcessed = true;
}

QString MessageAction::getRawMessage()
{
    return message;
}
