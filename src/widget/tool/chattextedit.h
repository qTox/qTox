/*
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
    ~ChatTextEdit();
    virtual void keyPressEvent(QKeyEvent * event) override;
    void setLastMessage(QString lm);
    
signals:
    void enterPressed();
    void tabPressed();
    void keyPressed();

private:
    void retranslateUi();

private:
    QString lastMessage;
};

#endif // CHATTEXTEDIT_H
