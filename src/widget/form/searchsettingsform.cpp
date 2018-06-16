#include "searchsettingsform.h"
#include "ui_searchsettingsform.h"
#include "src/widget/style.h"

SearchSettingsForm::SearchSettingsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SearchSettingsForm)
{
    ui->setupUi(this);

    ui->choiceDateButton->setEnabled(false);
    ui->startDateLabel->setEnabled(false);

    ui->choiceDateButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->choiceDateButton->setObjectName("choiceDateButton");
    ui->choiceDateButton->setStyleSheet(Style::getStylesheet(QStringLiteral(":/ui/chatForm/buttons.css")));

    ui->startDateLabel->setStyleSheet("QLabel{color: #ddd;}");

    connect(ui->startSearchComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &SearchSettingsForm::onStartSearchSelected);
    connect(ui->registerCheckBox, &QCheckBox::clicked, this, &SearchSettingsForm::onRegisterClicked);
    connect(ui->wordsOnlyCheckBox, &QCheckBox::clicked, this, &SearchSettingsForm::onWordsOnlyClicked);
    connect(ui->regularCheckBox, &QCheckBox::clicked, this, &SearchSettingsForm::onRegularClicked);
}

SearchSettingsForm::~SearchSettingsForm()
{
    delete ui;
}

void SearchSettingsForm::onStartSearchSelected(const int index)
{
    if (index > 1) {
        ui->choiceDateButton->setEnabled(true);
        ui->startDateLabel->setEnabled(true);

        ui->choiceDateButton->setProperty("state", "green");
        ui->choiceDateButton->setStyleSheet(Style::getStylesheet(QStringLiteral(":/ui/chatForm/buttons.css")));

        ui->startDateLabel->setStyleSheet("QLabel{color: #000;}");
    } else {
        ui->choiceDateButton->setEnabled(false);
        ui->startDateLabel->setEnabled(false);

        ui->choiceDateButton->setProperty("state", "");
        ui->choiceDateButton->setStyleSheet(Style::getStylesheet(QStringLiteral(":/ui/chatForm/buttons.css")));

        ui->startDateLabel->setStyleSheet("QLabel{color: #ddd;}");
    }
}

void SearchSettingsForm::onRegisterClicked(const bool checked)
{
    if (checked) {
        ui->regularCheckBox->setChecked(false);
    }
}

void SearchSettingsForm::onWordsOnlyClicked(const bool checked)
{
    if (checked) {
        ui->regularCheckBox->setChecked(false);
    }
}

void SearchSettingsForm::onRegularClicked(const bool checked)
{
    if (checked) {
        ui->registerCheckBox->setChecked(false);
        ui->wordsOnlyCheckBox->setChecked(false);
    }
}
