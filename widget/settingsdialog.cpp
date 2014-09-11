#include "settingsdialog.h"
#include "settings.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QStackedWidget>
#include <QPushButton>
#include <QBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QComboBox>

GeneralPage::GeneralPage(QWidget *parent)
{
    QGroupBox *configGroup = new QGroupBox(tr("Server configuration"));

    QLabel *serverLabel = new QLabel(tr("Server:"));
    QComboBox *serverCombo = new QComboBox;
    serverCombo->addItem(tr("Qt (Australia)"));
    serverCombo->addItem(tr("Qt (Germany)"));
    serverCombo->addItem(tr("Qt (Norway)"));
    serverCombo->addItem(tr("Qt (People's Republic of China)"));
    serverCombo->addItem(tr("Qt (USA)"));

    QHBoxLayout *serverLayout = new QHBoxLayout;
    serverLayout->addWidget(serverLabel);
    serverLayout->addWidget(serverCombo);

    QVBoxLayout *configLayout = new QVBoxLayout;
    configLayout->addLayout(serverLayout);
    configGroup->setLayout(configLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(configGroup);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

Identity::Identity(QWidget *parent)
{

}

Privacy::Privacy(QWidget *parent)
{

}




SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent)
{
    contentsWidget = new QListWidget;
    contentsWidget->setViewMode(QListView::IconMode);
    contentsWidget->setIconSize(QSize(64, 64));
    contentsWidget->setMovement(QListView::Static);
    contentsWidget->setMaximumWidth(85);
    contentsWidget->setMinimumWidth(85);
    contentsWidget->setMinimumHeight(300);
    contentsWidget->setSpacing(7);

    pagesWidget = new QStackedWidget;
    pagesWidget->addWidget(new GeneralPage(this));
    pagesWidget->addWidget(new Identity(this));
    pagesWidget->addWidget(new Privacy(this));

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
{
    QListWidgetItem *generalButton = new QListWidgetItem(contentsWidget);
    generalButton->setIcon(QIcon(":/img/settings/general.png"));
    generalButton->setText(tr("General"));
    generalButton->setTextAlignment(Qt::AlignHCenter);
    generalButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *identity = new QListWidgetItem(contentsWidget);
    identity->setIcon(QIcon(":/img/settings/identity.png"));
    identity->setText(tr("Identity"));
    identity->setTextAlignment(Qt::AlignHCenter);
    identity->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *privacy = new QListWidgetItem(contentsWidget);
    privacy->setIcon(QIcon(":/img/settings/privacy.png"));
    privacy->setText(tr("Privacy"));
    privacy->setTextAlignment(Qt::AlignHCenter);
    privacy->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    connect(contentsWidget,
             SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
             this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));
}

void SettingsDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current) {
         current = previous;
    }
    pagesWidget->setCurrentIndex(contentsWidget->row(current));
}

void SettingsDialog::readConfig()
{
    Settings& settings = Settings::getInstance();
}

void SettingsDialog::writeConfig()
{}
