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

#ifndef CHATTEXTEDIT_H
#define CHATTEXTEDIT_H

#include <QTextEdit>

class ChatTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit ChatTextEdit(QWidget *parent = 0);
    virtual void keyPressEvent(QKeyEvent * event) override;

signals:
    void enterPressed();

public slots:

};

#endif // CHATTEXTEDIT_H
