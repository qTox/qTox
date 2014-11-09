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

#include "filetransferaction.h"
#include "src/filetransferinstance.h"

#include <QTextEdit>
#include <QScrollBar>

FileTransferAction::FileTransferAction(FileTransferInstance *widget, const QString &author, const QString &date, const bool &me)
  : ChatAction(me, author, date)
{
    w = widget;

    connect(w, &FileTransferInstance::stateUpdated, this, &FileTransferAction::updateContent);
}

FileTransferAction::~FileTransferAction()
{
}

QString FileTransferAction::getMessage()
{
    QString widgetHtml;
    if (w != nullptr)
        widgetHtml = w->getHtmlImage();
    else
        widgetHtml = "<div class=quote>EMPTY CONTENT</div>";
    return widgetHtml;
}

/*
void FileTransferAction::setup(QTextCursor cursor, QTextEdit *textEdit)
{
    cur = cursor;
    cur.setKeepPositionOnInsert(true);
    int end=cur.selectionEnd();
    cur.setPosition(cur.position());
    cur.setPosition(end, QTextCursor::KeepAnchor);

    edit = textEdit;
}
*/
/*
void FileTransferAction::updateHtml()
{
    if (cur.isNull() || !edit)
        return;

    // save old slider value
    int vSliderVal = edit->verticalScrollBar()->value();

    // update content
    int pos = cur.selectionStart();
    cur.removeSelectedText();
    cur.setKeepPositionOnInsert(false);
    cur.insertHtml(getMessage());
    cur.setKeepPositionOnInsert(true);
    int end = cur.position();
    cur.setPosition(pos);
    cur.setPosition(end, QTextCursor::KeepAnchor);

    // restore old slider value
    edit->verticalScrollBar()->setValue(vSliderVal);
}
*/
bool FileTransferAction::isInteractive()
{
    if (w->getState() == FileTransferInstance::TransfState::tsCanceled
            || w->getState() == FileTransferInstance::TransfState::tsFinished)
    {
        return false;
    }

    return true;
}
