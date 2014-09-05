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

#ifndef EMOTICONSWIDGET_H
#define EMOTICONSWIDGET_H

#include <QMenu>
#include <QStackedWidget>
#include <QVBoxLayout>

class EmoticonsWidget : public QMenu
{
    Q_OBJECT
public:
    explicit EmoticonsWidget(QWidget *parent = 0);

signals:
    void insertEmoticon(QString str);

private slots:
    void onSmileyClicked();
    void onPageButtonClicked();

private:
    QStackedWidget stack;
    QVBoxLayout layout;

public:
    virtual QSize sizeHint() const;
};

#endif // EMOTICONSWIDGET_H
