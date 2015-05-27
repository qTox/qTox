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

#ifndef CIRCLEWIDGET_H
#define CIRCLEWIDGET_H

#include <QFrame>

class QVBoxLayout;
class QHBoxLayout;
class QLabel;

class CircleWidget : public QFrame
{
    Q_OBJECT
public:
    CircleWidget(QWidget *parent = 0);

    bool isCompact() const;
    void setCompact(bool compact);

    void toggle();

    Q_PROPERTY(bool compact READ isCompact WRITE setCompact)

protected:

    void mousePressEvent(QMouseEvent *event) override;

    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    enum FriendLayoutType
    {
        Online = 0,
        Offline = 1
    };
    bool compact, visible = false;
    QVBoxLayout *friendLayouts[2];
    QVBoxLayout *groupLayout;
    QVBoxLayout *mainLayout;
    QLabel *arrowLabel;
};

#endif // CIRCLEWIDGET_H
