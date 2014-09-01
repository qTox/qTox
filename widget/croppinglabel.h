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

#ifndef CROPPINGLABEL_H
#define CROPPINGLABEL_H

#include <QLabel>

class CroppingLabel : public QLabel
{
    Q_OBJECT
public:
    explicit CroppingLabel(QWidget *parent = 0);

    virtual void setText(const QString& text);
    virtual void resizeEvent(QResizeEvent *ev);
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

protected:
    void setElidedText();

private:
    QString origText;
};

#endif // CROPPINGLABEL_H
