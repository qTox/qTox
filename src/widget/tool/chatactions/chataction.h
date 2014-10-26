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

#ifndef CHATACTION_H
#define CHATACTION_H

#include <QString>
#include <QTextCursor>
#include <QSharedPointer>

class FileTransferInstance;
class QTextEdit;

class ChatAction : public QObject
{
public:
    ChatAction(const bool &me, const QString &author, const QString &date) : isMe(me), name(author), date(date) {;}
    virtual ~ChatAction(){;}
    virtual void setup(QTextCursor cursor, QTextEdit* textEdit) = 0; ///< Call once, and then you MUST let the object update itself

    virtual QString getName();
    virtual QString getMessage() = 0;
    virtual QString getDate();
    virtual bool isInteractive(){return false;}

protected:
    QString toHtmlChars(const QString &str);
    QString QImage2base64(const QImage &img);

protected:
    bool isMe;
    QString name, date;
};

typedef QSharedPointer<ChatAction> ChatActionPtr;

#endif // CHATACTION_H
