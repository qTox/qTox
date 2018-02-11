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

#include "searchform.h"
#include "src/widget/style.h"
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

SearchForm::SearchForm(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout();
    searchLine = new QLineEdit();
    upButton = new QPushButton();
    upButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    upButton->setObjectName("searchUpButton");
    upButton->setProperty("state", "green");
    upButton->setStyleSheet(Style::getStylesheet(QStringLiteral(":/ui/chatForm/buttons.css")));

    downButton = new QPushButton();
    downButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    downButton->setObjectName("searchDownButton");
    downButton->setProperty("state", "green");
    downButton->setStyleSheet(Style::getStylesheet(QStringLiteral(":/ui/chatForm/buttons.css")));

    hideButton = new QPushButton();
    hideButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    hideButton->setObjectName("hideButton");
    hideButton->setProperty("state", "red");
    hideButton->setStyleSheet(Style::getStylesheet(QStringLiteral(":/ui/chatForm/buttons.css")));

    layout->setMargin(0);
    layout->addWidget(searchLine);
    layout->addWidget(upButton);
    layout->addWidget(downButton);
    layout->addWidget(hideButton);

    setLayout(layout);

    connect(searchLine, &QLineEdit::textChanged, this, &SearchForm::changedSearchPhrase);
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

void SearchForm::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    emit visibleChanged();
}

void SearchForm::changedSearchPhrase(const QString &text)
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
