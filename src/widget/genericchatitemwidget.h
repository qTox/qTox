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

#ifndef GENERICCHATITEMWIDGET_H
#define GENERICCHATITEMWIDGET_H

#include <QFrame>

class CroppingLabel;

class GenericChatItemWidget : public QFrame
{
    Q_OBJECT
public:
    GenericChatItemWidget(QWidget *parent = 0);

    bool isCompact() const;
    void setCompact(bool compact);

    QString getName() const;

    Q_PROPERTY(bool compact READ isCompact WRITE setCompact)

protected:
    CroppingLabel* nameLabel;

private:
    bool compact;
};

#endif // GENERICCHATITEMWIDGET_H
