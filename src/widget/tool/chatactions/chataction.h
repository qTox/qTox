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
class QTextTable;

class ChatAction : public QObject
{
    Q_OBJECT
public:
    ChatAction(const bool &me, const QString &author, const QString &date) : isMe(me), name(author), date(date) {;}
    virtual ~ChatAction(){;}

    void assignPlace(QTextTable *position, QTextEdit* te);
    virtual void dispaly();
    virtual bool isInteractive(){return false;}
    virtual void featureUpdate() {;}

    static void setupFormat();

public slots:
    void updateContent();

protected:
    virtual QString getName();
    virtual QString getMessage() = 0;
    virtual QString getDate();

    QString toHtmlChars(const QString &str);
    QString QImage2base64(const QImage &img);

protected:
    bool isMe;
    QString name, date;

    QTextTable *textTable;
    QTextEdit *textEdit;
    QTextCursor cur;

    static QTextBlockFormat nameFormat, dateFormat;
};

typedef QSharedPointer<ChatAction> ChatActionPtr;

#endif // CHATACTION_H
