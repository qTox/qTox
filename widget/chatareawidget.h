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

#ifndef CHATAREAWIDGET_H
#define CHATAREAWIDGET_H

#include <QTextBrowser>
#include <QList>
#include <QTextTable>

class ChatAction;

class ChatAreaWidget : public QTextBrowser
{
    Q_OBJECT
public:
    explicit ChatAreaWidget(QWidget *parent = 0);
    virtual ~ChatAreaWidget();
    void insertMessage(ChatAction *msgAction);

signals:
    void onFileTranfertInterract(QString widgetName, QString buttonName);

protected:
    void mouseReleaseEvent(QMouseEvent * event);

private slots:
    void onAnchorClicked(const QUrl& url);
    void onSliderRangeChanged();

private:
    void checkSlider();

    QList<ChatAction*> messages;
    bool lockSliderToBottom;
    int sliderPosition;
    QTextTable *chatTextTable;
    QTextBlockFormat nameFormat, dateFormat;
};

#endif // CHATAREAWIDGET_H
