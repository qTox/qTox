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
}
