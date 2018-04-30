/*
    Copyright © 2015-2018 by The qTox Project Contributors

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
#include <QLineEdit>

class QPushButton;
class LineEdit;

class SearchForm final : public QWidget
{
    Q_OBJECT
public:
    explicit SearchForm(QWidget* parent = nullptr);
    void removeSearchPhrase();
    QString getSearchPhrase() const;
    void setFocusEditor();
    void insertEditor(const QString &text);

protected:
    virtual void showEvent(QShowEvent* event) final override;

private:
    // TODO: Merge with 'createButton' from chatformheader.cpp
    QPushButton* createButton(const QString& name, const QString& state);

    QPushButton* upButton;
    QPushButton* downButton;
    QPushButton* hideButton;
    LineEdit* searchLine;

    QString searchPhrase;

private slots:
    void changedSearchPhrase(const QString& text);
    void clickedUp();
    void clickedDown();
    void clickedHide();

signals:
    void searchInBegin(const QString& phrase);
    void searchUp(const QString& phrase);
    void searchDown(const QString& phrase);
    void visibleChanged();
};

class LineEdit : public QLineEdit
{
    Q_OBJECT

public:
    LineEdit(QWidget* parent = nullptr);

protected:
    virtual void keyPressEvent(QKeyEvent* event) final override;

signals:
    void clickEnter();
    void clickShiftEnter();
    void clickEsc();
};

#endif // SEARCHFORM_H
