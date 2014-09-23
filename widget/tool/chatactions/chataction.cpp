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

QString ChatAction::getName()
{
    if (isMe)
        return QString("<div class=name_me>" + toHtmlChars(name) + "</div>");
    else
        return QString("<div class=name>" + toHtmlChars(name) + "</div>");
}

QString ChatAction::getDate()
{
    QString res = date;
    return res;
}
