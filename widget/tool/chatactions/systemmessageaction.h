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

#ifndef SYSTEMMESSAGEACTION_H
#define SYSTEMMESSAGEACTION_H

#include "widget/tool/chatactions/chataction.h"

class SystemMessageAction : public ChatAction
{
public:
    SystemMessageAction(const QString &message, const QString& type, const QString &date);
    virtual ~SystemMessageAction(){;}
    virtual void setup(QTextCursor, QTextEdit*) override {;}

    virtual QString getName() {return QString();}
    virtual QString getMessage();

private:
    QString message;
    QString type;
};

#endif // SYSTEMMESSAGEACTION_H
