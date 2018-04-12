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

#include "searchform.h"
#include "src/widget/style.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QKeyEvent>

SearchForm::SearchForm(QWidget* parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout();
    searchLine = new LineEdit();

    upButton = createButton("searchUpButton", "green");
    downButton = createButton("searchDownButton", "green");
    hideButton = createButton("searchHideButton", "red");

    layout->setMargin(0);
    layout->addWidget(searchLine);
    layout->addWidget(upButton);
    layout->addWidget(downButton);
    layout->addWidget(hideButton);

    setLayout(layout);

    connect(searchLine, &LineEdit::textChanged, this, &SearchForm::changedSearchPhrase);
    connect(searchLine, &LineEdit::clickEnter, this, &SearchForm::clickedUp);
    connect(searchLine, &LineEdit::clickShiftEnter, this, &SearchForm::clickedDown);
    connect(searchLine, &LineEdit::clickEsc, this, &SearchForm::clickedHide);

    connect(upButton, &QPushButton::clicked, this, &SearchForm::clickedUp);
    connect(downButton, &QPushButton::clicked, this, &SearchForm::clickedDown);
    connect(hideButton, &QPushButton::clicked, this, &SearchForm::clickedHide);
}

void SearchForm::removeSearchPhrase()
{
    searchLine->setText("");
}

QString SearchForm::getSearchPhrase() const
{
    return searchPhrase;
}

void SearchForm::setFocusEditor()
{
    searchLine->setFocus();
}

void SearchForm::insertEditor(const QString &text)
{
    searchLine->insert(text);
}

void SearchForm::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    emit visibleChanged();
}

QPushButton *SearchForm::createButton(const QString& name, const QString& state)
{
    QPushButton* btn = new QPushButton();
    btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    btn->setObjectName(name);
    btn->setProperty("state", state);
    btn->setStyleSheet(Style::getStylesheet(QStringLiteral(":/ui/chatForm/buttons.css")));

    return btn;
}

void SearchForm::changedSearchPhrase(const QString& text)
{
    searchPhrase = text;
    emit searchInBegin(searchPhrase);
}

void SearchForm::clickedUp()
{
    emit searchUp(searchPhrase);
}

void SearchForm::clickedDown()
{
    emit searchDown(searchPhrase);
}

void SearchForm::clickedHide()
{
    hide();
    emit visibleChanged();
}

LineEdit::LineEdit(QWidget* parent) : QLineEdit(parent)
{
}

void LineEdit::keyPressEvent(QKeyEvent* event)
{
    int key = event->key();

    if ((key == Qt::Key_Enter || key == Qt::Key_Return)) {
        if ((event->modifiers() & Qt::ShiftModifier)) {
            emit clickShiftEnter();
        } else {
            emit clickEnter();
        }
    } else if (key == Qt::Key_Escape) {
        emit clickEsc();
    }

    QLineEdit::keyPressEvent(event);
}


