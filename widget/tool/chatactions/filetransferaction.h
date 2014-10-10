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

#ifndef FILETRANSFERACTION_H
#define FILETRANSFERACTION_H

#include "widget/tool/chatactions/chataction.h"

class FileTransferAction : public ChatAction
{
    Q_OBJECT
public:
    FileTransferAction(FileTransferInstance *widget, const QString &author, const QString &date, const bool &me);
    virtual ~FileTransferAction();
    virtual QString getMessage();
    virtual void setup(QTextCursor cursor, QTextEdit* textEdit) override;
    virtual bool isInteractive();

private slots:
    void updateHtml();

private:
    FileTransferInstance *w;
    QTextCursor cur;
    QTextEdit* edit;
};

#endif // FILETRANSFERACTION_H
