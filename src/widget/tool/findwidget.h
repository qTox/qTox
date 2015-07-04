/*
    Copyright © 2015 by The qTox Project

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

#ifndef FINDWIDGET_H
#define FINDWIDGET_H

#include <QWidget>

class QLabel;

class FindWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FindWidget(QWidget *parent = 0);

signals:
    void findText(const QString& text);
    void findNext(const QString& text, int index, int total);
    void findPrevious(const QString& text, int index, int total);
    void setCase(bool match);
    void close();

public slots:
    void setMatches(int index, int matches);

private:
    QLabel* matchesLabel;
    int index;
    int total;
};

#endif // FINDWIDGET_H
