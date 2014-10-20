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
#include <QDebug>

ActionAction::ActionAction(const QString &author, QString message, const QString &date, const bool& me) :
    MessageAction(author, author+" "+message, date, me)
{
}

void ActionAction::setup(QTextCursor cursor, QTextEdit *)
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

QString ActionAction::getName()
{
    return QString("<div class=action>*</div>");
}

QString ActionAction::getMessage()
{
    return MessageAction::getMessage("action");
}
