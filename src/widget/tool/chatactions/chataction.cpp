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

#include "chataction.h"
#include <QStringList>
#include <QBuffer>
#include <QTextTable>
#include <QScrollBar>
#include <QTextEdit>

QTextBlockFormat ChatAction::nameFormat, ChatAction::dateFormat;

QString ChatAction::toHtmlChars(const QString &str)
{
    static QList<QPair<QString, QString>> replaceList = {{"&","&amp;"}, {">","&gt;"}, {"<","&lt;"}, {" ", "&nbsp;"}}; // {"&","&amp;"} should be always first
    QString res = str;

    for (auto &it : replaceList)
        res = res.replace(it.first,it.second);

    return res;
}

QString ChatAction::QImage2base64(const QImage &img)
{
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "PNG"); // writes image into ba in PNG format
    return ba.toBase64();
}

QString ChatAction::getName()
{
    if (isMe)
        return QString("<div class=%1>%2</div>").arg("name_me").arg(toHtmlChars(name));
    else
        return QString("<div class=%1>%2</div>").arg("name").arg(toHtmlChars(name));
}

QString ChatAction::getDate()
{
    if (isMe)
        return QString("<div class=date_me>" + toHtmlChars(date) + "</div>");
    else
        return QString("<div class=date>" + toHtmlChars(date) + "</div>");
}

void ChatAction::assignPlace(QTextTable *position, QTextEdit *te)
{
    textTable = position;
    cur = position->cellAt(0, 2).firstCursorPosition();
    cur.clearSelection();
    cur.setKeepPositionOnInsert(true);
    textEdit = te;
}

void ChatAction::dispaly()
{
    textTable->cellAt(0, 0).firstCursorPosition().setBlockFormat(nameFormat);
    textTable->cellAt(0, 0).firstCursorPosition().insertHtml(getName());
    textTable->cellAt(0, 2).firstCursorPosition().insertHtml(getMessage());
    textTable->cellAt(0, 4).firstCursorPosition().setBlockFormat(dateFormat);
    textTable->cellAt(0, 4).firstCursorPosition().insertHtml(getDate());

    cur.setKeepPositionOnInsert(true);
    int end=cur.selectionEnd();
    cur.setPosition(cur.position());
    cur.setPosition(end, QTextCursor::KeepAnchor);

    featureUpdate();
}

void ChatAction::setupFormat()
{
    nameFormat.setAlignment(Qt::AlignRight);
    nameFormat.setNonBreakableLines(true);
    dateFormat.setAlignment(Qt::AlignLeft);
    dateFormat.setNonBreakableLines(true);
}

void ChatAction::updateContent()
{
    if (cur.isNull() || !textEdit)
        return;

    int vSliderVal = textEdit->verticalScrollBar()->value();

    // update content
    int pos = cur.selectionStart();
    cur.removeSelectedText();
    cur.setKeepPositionOnInsert(false);
    cur.insertHtml(getMessage());
    cur.setKeepPositionOnInsert(true);
    int end = cur.position();
    cur.setPosition(pos);
    cur.setPosition(end, QTextCursor::KeepAnchor);

    // restore old slider value
    textEdit->verticalScrollBar()->setValue(vSliderVal);

    featureUpdate();
}
