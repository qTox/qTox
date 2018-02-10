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

    layout->addWidget(searchLine);
    layout->addWidget(upButton);
    layout->addWidget(downButton);

    setLayout(layout);

    connect(searchLine, &QLineEdit::textChanged, this, &SearchForm::changedSearchPhrare);
    connect(upButton, &QPushButton::clicked, this, &SearchForm::clickedUp);
    connect(downButton, &QPushButton::clicked, this, &SearchForm::clickedDown);
}

void SearchForm::removeText()
{
    searchLine->setText("");
}

void SearchForm::changedSearchPhrare(const QString &text)
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
