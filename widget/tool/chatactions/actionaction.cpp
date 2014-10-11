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

#include "actionaction.h"
#include "misc/smileypack.h"

ActionAction::ActionAction(const QString &author, const QString &message, const QString &date, const bool& me) :
    ChatAction(me, author, date),
    message(message)
{
}

QString ActionAction::getName()
{
    return QString("<div class=action>*</div>");
}

QString ActionAction::getMessage()
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

        return QString("<div class=action>%1 %2</div>").arg(name).arg(message_);
}
