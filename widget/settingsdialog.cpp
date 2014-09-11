#include "settingsdialog.h"
#include "settings.h"
#include "widget.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QStackedWidget>
#include <QPushButton>
#include <QBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>
#include <QLineEdit>

General::General(QWidget *parent) :
    QWidget(parent)
{
    QGroupBox *group = new QGroupBox(tr("General Settings"), this);

    enableIPv6 = new QCheckBox(this);
    enableIPv6->setText(tr("Enable IPv6 (recommended)","Text on a checkbox to enable IPv6"));

    useTranslations = new QCheckBox(this);
    useTranslations->setText(tr("Use translations","Text on a checkbox to enable translations"));

    makeToxPortable = new QCheckBox(this);
    makeToxPortable->setText(tr("Make Tox portable","Text on a checkbox to make qTox a portable application"));
    makeToxPortable->setToolTip(tr("Save settings to the working directory instead of the usual conf dir","describes makeToxPortable checkbox"));

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(enableIPv6);
    vLayout->addWidget(useTranslations);
    vLayout->addWidget(makeToxPortable);
    group->setLayout(vLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(group);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

Identity::Identity(QWidget *parent) :
    QWidget(parent)
{
    QGroupBox *group = new QGroupBox(tr("Public Information"), this);

    QLabel* nameLabel = new QLabel(tr("Name","Username/nick"), this);
    name = new QLineEdit(this);
    QLabel* statusLabel = new QLabel(tr("Status","Status message"));
    status = new QLineEdit(this);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(nameLabel);
    vLayout->addWidget(name);
    vLayout->addWidget(statusLabel);
    vLayout->addWidget(status);
    group->setLayout(vLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(group);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

Privacy::Privacy(QWidget *parent) :
    QWidget(parent)
{

}




SettingsDialog::SettingsDialog(Widget *parent) :
    QDialog(parent),
    widget(parent)
{
    createPages();
    createButtons();
    createConnections();

    setWindowTitle(tr("Settings Dialog"));
    setMinimumSize(600, 400);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(cancelButton);
    buttonsLayout->addWidget(applyButton);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(contentsWidget);
    hLayout->addWidget(pagesWidget, 1);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(hLayout);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);
}

void SettingsDialog::createPages()
{
    generalPage  = new General(this);
    identityPage = new Identity(this);
    privacyPage  = new Privacy(this);

    contentsWidget = new QListWidget;
    contentsWidget->setViewMode(QListView::IconMode);
    contentsWidget->setIconSize(QSize(64, 64));
    contentsWidget->setMovement(QListView::Static);
    contentsWidget->setMaximumWidth(90);
    contentsWidget->setMinimumWidth(90);
    contentsWidget->setSpacing(7);

    pagesWidget = new QStackedWidget;
    pagesWidget->addWidget(generalPage);
    pagesWidget->addWidget(identityPage);
    pagesWidget->addWidget(privacyPage);

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

    contentsWidget->setCurrentRow(0);
}

void SettingsDialog::createButtons()
{
    okButton     = new QPushButton(tr("Ok"), this);
    cancelButton = new QPushButton(tr("Cancel"), this);
    applyButton  = new QPushButton(tr("Apply"), this);
}

void SettingsDialog::createConnections()
{
    connect(okButton, SIGNAL(clicked()), this, SLOT(okPressed()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(applyButton, SIGNAL(clicked()), this, SLOT(applyPressed()));
    connect(
        contentsWidget,
        SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
        this,
        SLOT(changePage(QListWidgetItem*,QListWidgetItem*))
    );
}

void SettingsDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current) {
        current = previous;
    }
    pagesWidget->setCurrentIndex(contentsWidget->row(current));
}

void SettingsDialog::okPressed()
{
    writeConfig();
    close();
}

void SettingsDialog::cancelPressed()
{
    close();
}

void SettingsDialog::applyPressed()
{
    writeConfig();
}

void SettingsDialog::readConfig()
{
    Settings& settings = Settings::getInstance();

    generalPage->enableIPv6->setChecked(settings.getEnableIPv6());
    generalPage->useTranslations->setChecked(settings.getUseTranslations());
    generalPage->makeToxPortable->setChecked(settings.getMakeToxPortable());

    identityPage->name->setText("test name");
    identityPage->status->setText("test status");
}

void SettingsDialog::writeConfig()
{
    Settings& settings = Settings::getInstance();
    settings.setEnableIPv6(generalPage->enableIPv6->isChecked());
    settings.setUseTranslations(generalPage->useTranslations->isChecked());
    settings.setMakeToxPortable(generalPage->makeToxPortable->isChecked());
    settings.save();
}
