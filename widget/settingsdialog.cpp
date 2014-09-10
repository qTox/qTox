#include "settingsdialog.h"
#include "settings.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QStackedWidget>
#include <QPushButton>
#include <QBoxLayout>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent)
{
    contentsWidget = new QListWidget;
    contentsWidget->setViewMode(QListView::IconMode);
    contentsWidget->setIconSize(QSize(96, 84));
    contentsWidget->setMovement(QListView::Static);
    contentsWidget->setMaximumWidth(128);
    contentsWidget->setSpacing(12);

    pagesWidget = new QStackedWidget;
    //pagesWidget->addWidget(new ConfigurationPage);
    //pagesWidget->addWidget(new UpdatePage);
    //pagesWidget->addWidget(new QueryPage);

    QPushButton *closeButton = new QPushButton(tr("Close"));

    createIcons();
    contentsWidget->setCurrentRow(0);

    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(contentsWidget);
    horizontalLayout->addWidget(pagesWidget, 1);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(closeButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(horizontalLayout);
    mainLayout->addStretch(1);
    mainLayout->addSpacing(12);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);

    setWindowTitle(tr("Config Dialog"));
}

void SettingsDialog::createIcons()
{}

void SettingsDialog::readConfig()
{
    Settings& settings = Settings::getInstance();
}

void SettingsDialog::writeConfig()
{}

void SettingsDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{}
