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

#include "findwidget.h"
#include <QBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>

FindWidget::FindWidget(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout* findLayout = new QHBoxLayout(this);

    matchesLabel = new QLabel();
    findLayout->addWidget(matchesLabel, 1);
    setMatches(0);

    QLineEdit* lineEdit = new QLineEdit(this);
    lineEdit->setFocus();
    findLayout->addWidget(lineEdit, 1);

    QPushButton* previousButton = new QPushButton();
    previousButton->setToolTip(tr("Find the previous occurence of the phrase"));
    previousButton->setIcon(QIcon("://ui/chatArea/scrollBarUpArrow.svg"));
    findLayout->addWidget(previousButton);

    QPushButton* nextButton = new QPushButton();
    nextButton ->setToolTip(tr("Find the next occurence of the phrase"));
    nextButton ->setIcon(QIcon("://ui/chatArea/scrollBarDownArrow.svg"));
    findLayout->addWidget(nextButton);

    QCheckBox* matchCheck = new QCheckBox(tr("Match Case"));
    findLayout->addWidget(matchCheck);

    QPushButton* closeButton = new QPushButton(this);
    closeButton->setIcon(QIcon("://ui/fileTransferInstance/no.svg"));
    closeButton->setToolTip(tr("Close the find bar"));
    findLayout->addWidget(closeButton);

    connect(lineEdit, &QLineEdit::textChanged, this, &FindWidget::findText);
    connect(nextButton, &QPushButton::pressed, this, &FindWidget::findNext);
    connect(previousButton, &QPushButton::pressed, this, &FindWidget::findPrevious);
    connect(closeButton, &QPushButton::pressed, this, &FindWidget::close);
}

void FindWidget::setMatches(int matches)
{
    if (matches <= 0)
        matchesLabel->setText(QString());
    else if (matches > 100)
        matchesLabel->setText(tr("More than 100 matches"));
    else
        matchesLabel->setText(tr("%1 of %2 matches").arg(matches));
}

