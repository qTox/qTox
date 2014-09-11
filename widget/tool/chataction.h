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

class FileTransferInstance;

class ChatAction : public QObject
{
public:
    ChatAction(const bool &me, const QString &author, const QString &date) : isMe(me), name(author), date(date) {;}
    virtual ~ChatAction(){;}
    virtual void setTextCursor(QTextCursor cursor){(void)cursor;} ///< Call once, and then you MUST let the object update itself

    virtual QString getName();
    virtual QString getMessage() = 0;
    virtual QString getDate();

protected:
    QString toHtmlChars(const QString &str);
    QString QImage2base64(const QImage &img);

protected:
    bool isMe;
    QString name, date;
};

class MessageAction : public ChatAction
{
public:
    MessageAction(const QString &author, const QString &message, const QString &date, const bool &me);
    virtual ~MessageAction(){;}
    virtual QString getMessage();
    virtual void setTextCursor(QTextCursor cursor) final;

private:
    QString message;
};

class FileTransferAction : public ChatAction
{
    Q_OBJECT
public:
    FileTransferAction(FileTransferInstance *widget, const QString &author, const QString &date, const bool &me);
    virtual ~FileTransferAction();
    virtual QString getMessage();
    virtual void setTextCursor(QTextCursor cursor) final;

private slots:
    void updateHtml();

private:
    FileTransferInstance *w;
    QTextCursor cur;
};

#endif // CHATACTION_H
