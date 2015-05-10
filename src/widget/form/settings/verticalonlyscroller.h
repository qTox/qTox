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

#ifndef VERTICALONLYSCROLLER_H
#define VERTICALONLYSCROLLER_H

#include <QScrollArea>

class QResizeEvent;
class QShowEvent;

class VerticalOnlyScroller : public QScrollArea
{
    Q_OBJECT
public:
    explicit VerticalOnlyScroller(QWidget *parent = 0);

protected:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};

#endif // VERTICALONLYSCROLLER_H
