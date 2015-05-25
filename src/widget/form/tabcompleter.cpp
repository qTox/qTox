/*
    Copyright (C) 2005-2014 by the Quassel Project
    devel@quassel-irc.org

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

/* This file was taken from the Quassel IRC client source (src/uisupport), and
   was greatly simplified for use in qTox. */

#include "tabcompleter.h"
#include "src/core/core.h"
#include "src/group.h"
#include "src/widget/tool/chattextedit.h"
#include <QRegExp>
#include <QKeyEvent>

const QString TabCompleter::nickSuffix = QString(": ");

TabCompleter::TabCompleter(ChatTextEdit* msgEdit, Group* group)
    : QObject{msgEdit}, msgEdit{msgEdit}, group{group},
      enabled{false}, lastCompletionLength{0}
{
}

/* from quassel/src/uisupport/multilineedit.h
    // Compatibility methods with the rest of the classes which still expect this to be a QLineEdit
    inline QString text() const { return toPlainText(); }
    inline QString html() const { return toHtml(); }
    inline int cursorPosition() const { return textCursor().position(); }
    inline void insert(const QString &newText) { insertPlainText(newText); }
    inline void backspace() { keyPressEvent(new QKeyEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier)); }
*/

void TabCompleter::buildCompletionList()
{
    // ensure a safe state in case we return early.
    completionMap.clear();
    nextCompletion = completionMap.begin();

    // split the string on the given RE (not chars, nums or braces/brackets) and take the last section
    QString tabAbbrev = msgEdit->toPlainText().left(msgEdit->textCursor().position()).section(QRegExp("[^\\w\\d:--_\\[\\]{}|`^.\\\\]"), -1, -1);
    // that section is then used as the completion regex
    QRegExp regex(QString("^[-_\\[\\]{}|`^.\\\\]*").append(QRegExp::escape(tabAbbrev)), Qt::CaseInsensitive);

    for (auto name : group->getPeerList())
    {
        if (regex.indexIn(name) > -1)
            completionMap[name.toLower()] = name;
    }

    nextCompletion = completionMap.begin();
    lastCompletionLength = tabAbbrev.length();
}


void TabCompleter::complete()
{
    if (!enabled)
    {
        buildCompletionList();
        enabled = true;
    }

    if (nextCompletion != completionMap.end())
    {
        // clear previous completion
        auto cur = msgEdit->textCursor();
        cur.setPosition(cur.selectionEnd());
        msgEdit->setTextCursor(cur);
        for (int i = 0; i < lastCompletionLength; i++)
            msgEdit->textCursor().deletePreviousChar();

        // insert completion
        msgEdit->insertPlainText(*nextCompletion);

        // remember charcount to delete next time and advance to next completion
        lastCompletionLength = nextCompletion->length();
        nextCompletion++;

        // we're completing the first word of the line
        if (msgEdit->textCursor().position() == lastCompletionLength)
        {
            msgEdit->insertPlainText(nickSuffix);
            lastCompletionLength += nickSuffix.length();
        }
    }
    else
    { // we're at the end of the list -> start over again
        if (!completionMap.isEmpty())
        {
            nextCompletion = completionMap.begin();
            complete();
        }
    }
}

void TabCompleter::reset()
{
    enabled = false;
}

// this determines the sort order
bool TabCompleter::SortableString::operator<(const SortableString &other) const
{
    QString name = Core::getInstance()->getUsername();
    if (this->contents == name)
        return false;
    else if (other.contents == name)
        return true;

/*  QDateTime thisTime = thisUser->lastChannelActivity(_currentBufferId);
    QDateTime thatTime = thatUser->lastChannelActivity(_currentBufferId);


    if (thisTime.isValid() || thatTime.isValid())
        return thisTime > thatTime;
*/ // this could be a useful feature at some point

    return QString::localeAwareCompare(this->contents, other.contents) < 0;
}
