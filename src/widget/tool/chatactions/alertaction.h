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

#ifndef ALERTACTION_H
#define ALERTACTION_H

#include "messageaction.h"

class AlertAction : public MessageAction
{
public:
    AlertAction(const QString &author, const QString &message, const QString& date);
    virtual ~AlertAction(){;}
    virtual QString getMessage();
    //virtual QString getName(); only do the message for now; preferably would do the whole row
    virtual void setup(QTextCursor cursor, QTextEdit*) override;

private:
    QString message;
};

#endif // MESSAGEACTION_H
