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
#include "smileypack.h"
#include <QStringList>
#include <QBuffer>
#include "filetransferinstance.h"

QString ChatAction::toHtmlChars(const QString &str)
{
    static QList<QPair<QString, QString>> replaceList = {{"&","&amp;"}, {">","&gt;"}, {"<","&lt;"}};
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

QString ChatAction::wrapName(const QString &name)
{
    if (isMe)
        return QString("<td><div class=name_me>" + name + "</div></td>\n");
    else
        return QString("<td><div class=name>" + name + "</div></td>\n");
}

QString ChatAction::wrapDate(const QString &date)
{
    QString res = "<td align=right><div class=date>" + date + "</div></td>\n";
    return res;
}

QString ChatAction::wrapMessage(const QString &message)
{
    QString res = "<td width=100%><div class=message>" + message + "</div></td>\n";
    return res;
}

QString ChatAction::wrapWholeLine(const QString &line)
{
    QString res = "<tr>\n" + line + "</tr>\n";
    return res;
}

MessageAction::MessageAction(const QString &author, const QString &message, const QString &date, const bool &me) :
    ChatAction(me)
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
            message_ += "<div class=quote>" + s.right(s.length()-4) + "</div><br>";
        else
            message_ += s + "<br>";
    }
    message_ = message_.left(message_.length()-4);

    content = wrapWholeLine(wrapName(author) + wrapMessage(message_) + wrapDate(date));
}

void MessageAction::setTextCursor(QTextCursor cursor)
{
    // When this function is called, we're supposed to only update ourselve when needed
    // Nobody should ask us to do anything with our content, we're on our own
    // Except we never udpate on our own, so we can safely free our resources

    (void) cursor;
    content.clear();
    content.squeeze();
}

QString MessageAction::getHtml()
{
    return content;
}

FileTransferAction::FileTransferAction(FileTransferInstance *widget, const QString &author, const QString &date, const bool &me) :
    ChatAction(me),
    sender(author),
    timestamp(date)
{
    w = widget;

    connect(w, &FileTransferInstance::stateUpdated, this, &FileTransferAction::updateHtml);
}

FileTransferAction::~FileTransferAction()
{
}

QString FileTransferAction::getHtml()
{
    QString widgetHtml;
    if (w != nullptr)
        widgetHtml = w->getHtmlImage();
    else
        widgetHtml = "<div class=quote>EMPTY CONTENT</div>";
    QString res = wrapWholeLine(wrapName(sender) + wrapMessage(widgetHtml) + wrapDate(timestamp));
    return res;
}

QString FileTransferAction::wrapMessage(const QString &message)
{
    QString res = "<td width=100%>" + message + "</td>\n";
    return res;
}
void FileTransferAction::setTextCursor(QTextCursor cursor)
{
    cur = cursor;
    cur.setKeepPositionOnInsert(true);
    int end=cur.selectionEnd();
    cur.setPosition(cur.position());
    cur.setPosition(end, QTextCursor::KeepAnchor);
}

void FileTransferAction::updateHtml()
{
    if (cur.isNull())
        return;

    int pos = cur.selectionStart();
    cur.removeSelectedText();
    cur.setKeepPositionOnInsert(false);
    cur.insertHtml(getHtml());
    cur.setKeepPositionOnInsert(true);
    int end = cur.position();
    cur.setPosition(pos);
    cur.setPosition(end, QTextCursor::KeepAnchor);

    // Free our ressources if we'll never need to update again
    if (w->getState() == FileTransferInstance::TransfState::tsCanceled
            || w->getState() == FileTransferInstance::TransfState::tsFinished)
    {
        sender.clear();
        sender.squeeze();
        timestamp.clear();
        timestamp.squeeze();
        cur = QTextCursor();
    }
}
