/*
    Copyright © 2016 by The qTox Project

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CONTENTWIDGET_H
#define CONTENTWIDGET_H

#include <QFrame>
#include <QPointer>

class QVBoxLayout;

class ContentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ContentWidget(QWidget *parent = nullptr,
                           Qt::WindowFlags f = Qt::WindowFlags());

    void setLayout(QLayout* layout) = delete;

    void setupLayout(QWidget* head, QWidget* body);
    QSize minimumSizeHint() const override;

protected:
    QWidget* headerWidget() const;
    QWidget* bodyWidget() const;

private:
    QVBoxLayout*        contentLayout;
    QFrame              mainHLine;
    QPointer<QWidget>   head;
    QPointer<QWidget>   body;
};

#endif
