/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

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

#include "emoticonswidget.h"
#include "src/misc/smileypack.h"
#include "src/misc/style.h"

#include <QPushButton>
#include <QRadioButton>
#include <QFile>
#include <QLayout>
#include <QGridLayout>

EmoticonsWidget::EmoticonsWidget(QWidget *parent) :
    QMenu(parent)
{
    setStyleSheet(Style::getStylesheet(":/ui/emoticonWidget/emoticonWidget.css"));
    setLayout(&layout);
    layout.addWidget(&stack);

    QWidget* pageButtonsContainer = new QWidget;
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    pageButtonsContainer->setLayout(buttonLayout);

    layout.addWidget(pageButtonsContainer);

    const int maxCols = 5;
    const int maxRows = 3;
    const int itemsPerPage = maxRows * maxCols;

    const QList<QStringList>& emoticons = SmileyPack::getInstance().getEmoticons();
    int itemCount = emoticons.size();
    int pageCount = (itemCount / itemsPerPage) + 1;
    int currPage = 0;
    int currItem = 0;
    int row = 0;
    int col = 0;

    // create pages
    buttonLayout->addStretch();
    for (int i = 0; i < pageCount; i++)
    {
        QGridLayout* pageLayout = new QGridLayout;
        pageLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), maxRows, 0);
        pageLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, maxCols);

        QWidget* page = new QWidget;
        page->setLayout(pageLayout);
        stack.addWidget(page);

        // page buttons are only needed if there is more than 1 page
        if (pageCount > 1)
        {
            QRadioButton* pageButton = new QRadioButton;
            pageButton->setProperty("pageIndex", i);
            pageButton->setCursor(Qt::PointingHandCursor);
            pageButton->setChecked(i == 0);
            buttonLayout->addWidget(pageButton);

            connect(pageButton, &QRadioButton::clicked, this, &EmoticonsWidget::onPageButtonClicked);
        }
    }
    buttonLayout->addStretch();

    for (const QStringList& set : emoticons)
    {
        QPushButton* button = new QPushButton;
        button->setIcon(SmileyPack::getInstance().getAsIcon(set[0]));
        button->setToolTip(set.join(" "));
        button->setProperty("sequence", set[0]);
        button->setCursor(Qt::PointingHandCursor);
        button->setFlat(true);

        connect(button, &QPushButton::clicked, this, &EmoticonsWidget::onSmileyClicked);

        qobject_cast<QGridLayout*>(stack.widget(currPage)->layout())->addWidget(button, row, col);

        col++;
        currItem++;

        // next row
        if (col >= maxCols)
        {
            col = 0;
            row++;
        }

        // next page
        if (currItem >= itemsPerPage)
        {
            row = 0;
            currItem = 0;
            currPage++;
        }
    }

    // calculates sizeHint
    layout.activate();
}

void EmoticonsWidget::onSmileyClicked()
{
    // hide the QMenu
    QMenu::hide();

    // emit insert emoticon
    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender)
        emit insertEmoticon(' ' + sender->property("sequence").toString() + ' ');
}

void EmoticonsWidget::onPageButtonClicked()
{
    QWidget* sender = qobject_cast<QRadioButton*>(QObject::sender());
    if (sender)
    {
        int page = sender->property("pageIndex").toInt();
        stack.setCurrentIndex(page);
    }
}

QSize EmoticonsWidget::sizeHint() const
{
    return layout.sizeHint();
}
