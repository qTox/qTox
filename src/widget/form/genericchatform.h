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

#ifndef GENERICCHATFORM_H
#define GENERICCHATFORM_H

#include <QWidget>
#include <QPoint>
#include <QDateTime>
#include <QMenu>
#include "src/widget/tool/chatactions/chataction.h"
#include "src/corestructs.h"

// Spacing in px inserted when the author of the last message changes
#define AUTHOR_CHANGE_SPACING 5 // why the hell is this a thing? surely the different font is enough?

class QLabel;
class QVBoxLayout;
class QPushButton;
class CroppingLabel;
class ChatTextEdit;
class ChatAreaWidget;
class MaskablePixmapWidget;
struct ToxID;

namespace Ui {
    class MainWindow;
}

class GenericChatForm : public QWidget
{
    Q_OBJECT
public:
    GenericChatForm(QWidget *parent = 0);

    virtual void setName(const QString &newName);
    virtual void show(Ui::MainWindow &ui);
    void addMessage(const QString& author, const QString &message, bool isAction, const QDateTime &datetime); ///< Deprecated
    void addMessage(const ToxID& author, const QString &message, bool isAction, const QDateTime &datetime);
    void addSelfMessage(const QString &message, bool isAction, const QDateTime &datetime);
    void addSystemInfoMessage(const QString &message, const QString &type, const QDateTime &datetime);
    void addAlertMessage(const QString& author, QString message, QDateTime datetime); ///< Deprecated
    void addAlertMessage(const ToxID& author, QString message, QDateTime datetime);
    int getNumberOfMessages();

signals:
    void sendMessage(int, QString);
    void sendAction(int, QString);

public slots:
    void focusInput();

protected slots:
    void onChatContextMenuRequested(QPoint pos);
    void onSaveLogClicked();
    void onEmoteButtonClicked();
    void onEmoteInsertRequested(QString str);
    void clearChatArea(bool);

protected:
    QString getElidedName(const QString& name);
    ChatActionPtr genMessageActionAction(const QString& author, QString message, bool isAction, const QDateTime &datetime); ///< Deprecated
    ChatActionPtr genMessageActionAction(const ToxID& author, QString message, bool isAction, const QDateTime &datetime);
    ChatActionPtr genSelfActionAction(QString message, bool isAction, const QDateTime &datetime);
    ChatActionPtr genSystemInfoAction(const QString &message, const QString &type, const QDateTime &datetime);

    ToxID previousId;
    QMenu menu;
    int curRow;
    CroppingLabel *nameLabel;
    MaskablePixmapWidget *avatar;
    QWidget *headWidget;
    QPushButton *fileButton, *emoteButton, *callButton, *videoButton, *volButton, *micButton;
    QVBoxLayout *headTextLayout;
    ChatTextEdit *msgEdit;
    QPushButton *sendButton;
    ChatAreaWidget *chatWidget;
    QDateTime *earliestMessage;
};

#endif // GENERICCHATFORM_H
