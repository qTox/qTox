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

#ifndef MESSAGEACTION_H
#define MESSAGEACTION_H

#include "chataction.h"

class MessageAction : public ChatAction
{
public:
    MessageAction(const QString &author, const QString &message, const QString &date, const bool &me);
    virtual ~MessageAction(){;}
    virtual void featureUpdate();
    void markAsSent();
    virtual QString getRawMessage();
    virtual bool isAction() {return false;}

protected:
    virtual QString getMessage();
    virtual QString getMessage(QString div);

protected:
    QString message;
    bool isProcessed;
};

typedef QSharedPointer<MessageAction> MessageActionPtr;

#endif // MESSAGEACTION_H
