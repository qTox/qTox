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
#include "src/widget/widget.h"
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

    // parse message
    message_ = Widget::parseMessage(message);

    // detect text quotes
    QStringList messageLines = message_.split("\n");
    message_ = "";
    for (QString& s : messageLines)
    {
        if (QRegExp("^[ ]*&gt;.*").exactMatch(s))
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
