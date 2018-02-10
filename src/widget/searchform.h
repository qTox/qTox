/*
    Copyright © 2015-2016 by The qTox Project Contributors

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

#ifndef SEARCHFORM_H
#define SEARCHFORM_H

#include <QWidget>

class QPushButton;
class QLineEdit;

class SearchForm final : public QWidget
{
    Q_OBJECT
public:
    explicit SearchForm(QWidget *parent = nullptr);
    void removeSearchPhrase();
    QString getSearchPhrase() const;

private:
    QPushButton* upButton;
    QPushButton* downButton;
    QLineEdit* searchLine;

    QString searchPhrase;

private slots:
    void changedSearchPhrare(const QString &text);
    void clickedUp();
    void clickedDown();

signals:
    void searchInBegin(const QString &);
    void searchUp(const QString &);
    void searchDown(const QString &);
};

#endif // SEARCHFORM_H
