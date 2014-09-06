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
#include "filetransferinstance.h"

class ChatAction
{
public:
    virtual ~ChatAction(){;}
    virtual QString getHtml() = 0;

protected:
    QString toHtmlChars(const QString &str);
    QString QImage2base64(const QImage &img);

    virtual QString wrapName(const QString &name);
    virtual QString wrapDate(const QString &date);
    virtual QString wrapMessage(const QString &message);
    virtual QString wrapWholeLine(const QString &line);
};

class MessageAction : public ChatAction
{
public:
    MessageAction(const QString &author, const QString &message, const QString &date);
    virtual ~MessageAction(){;}
    virtual QString getHtml();

private:
    QString content;
};

class FileTransferAction : public ChatAction
{
public:
    FileTransferAction(FileTransferInstance *widget, const QString &author, const QString &date);
    virtual ~FileTransferAction();
    virtual QString getHtml();
    virtual QString wrapMessage(const QString &message);

private:
    FileTransferInstance *w;
    QString sender, timestamp;
};

#endif // CHATACTION_H
