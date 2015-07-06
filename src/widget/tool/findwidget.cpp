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
#include "labeledlineedit.h"
#include <QBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>

FindWidget::FindWidget(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout* findLayout = new QHBoxLayout(this);

    QLabel* matchesLabel = new QLabel(tr("Find:"));
    findLayout->addWidget(matchesLabel);

    lineEdit = new LabeledLineEdit(this);
    lineEdit->setFocus();
    findLayout->addWidget(lineEdit, 1);

    QPushButton* previousButton = new QPushButton(tr("Previous"), this);
    previousButton->setToolTip(tr("Find the previous occurence of the phrase"));
    findLayout->addWidget(previousButton);

    QPushButton* nextButton = new QPushButton(tr("Next"), this);
    nextButton ->setToolTip(tr("Find the next occurence of the phrase"));
    findLayout->addWidget(nextButton);

    caseCheck = new QCheckBox(tr("Match Case"), this);
    findLayout->addWidget(caseCheck);

    QPushButton* closeButton = new QPushButton(tr("Close"), this);
    closeButton->setToolTip(tr("Close the find bar"));
    findLayout->addWidget(closeButton);

    connect(lineEdit, &QLineEdit::textChanged, this, &FindWidget::onFindText);
    connect(nextButton, &QPushButton::pressed, this, &FindWidget::onFindNextPressed);
    connect(previousButton, &QPushButton::pressed, this, &FindWidget::onFindPreviousPressed);
    connect(closeButton, &QPushButton::pressed, this, &FindWidget::onClosePressed);
    connect(caseCheck, &QCheckBox::clicked, this, &FindWidget::onFindText);
}

void FindWidget::setMatches(int index, int matches)
{
    total = matches;
    setIndex(index);
}

void FindWidget::setIndex(int newIndex)
{
    index = newIndex;

    if (lineEdit->text().isEmpty())
    {
        lineEdit->setLabelText(QString());
        lineEdit->setStyleSheet(QString());
    }
    else
    {
        lineEdit->setLabelText(tr("%1 of %2").arg(index).arg(total));

        if (total == 0)
            lineEdit->setStyleSheet("QLineEdit > QLabel {background-color: #ff6666;}");
        else
            lineEdit->setStyleSheet(QString());
    }
}

void FindWidget::onFindNextPressed()
{
    if (total != 0)
        emit findNext(lineEdit->text(), index + 1, total, caseCheck->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
}

void FindWidget::onFindPreviousPressed()
{
    if (total != 0)
        emit findPrevious(lineEdit->text(), index - 1, total, caseCheck->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
}

void FindWidget::onFindText()
{
    emit findText(lineEdit->text(), caseCheck->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
}

void FindWidget::onClosePressed()
{
    emit close(lineEdit->text());
}
