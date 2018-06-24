#include "searchsettingsform.h"
#include "ui_searchsettingsform.h"
#include "src/widget/style.h"
#include "src/widget/form/loadhistorydialog.h"

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
    isUpdate = false;

    connect(ui->startSearchComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &SearchSettingsForm::onStartSearchSelected);
    connect(ui->registerCheckBox, &QCheckBox::clicked, this, &SearchSettingsForm::onRegisterClicked);
    connect(ui->wordsOnlyRadioButton, &QCheckBox::clicked, this, &SearchSettingsForm::onWordsOnlyClicked);
    connect(ui->regularRadioButton, &QCheckBox::clicked, this, &SearchSettingsForm::onRegularClicked);
    connect(ui->choiceDateButton, &QPushButton::clicked, this, &SearchSettingsForm::onChoiceDate);
}

SearchSettingsForm::~SearchSettingsForm()
{
    delete ui;
}

ParameterSearch SearchSettingsForm::getParameterSearch()
{
    ParameterSearch ps;

    if (ui->regularRadioButton->isChecked()) {
        ps.filter = FilterSearch::Regular;
    } else if (ui->registerCheckBox->isChecked() && ui->wordsOnlyRadioButton->isChecked()) {
        ps.filter = FilterSearch::RegisterAndWordsOnly;
    } else if (ui->registerCheckBox->isChecked() && ui->regularRadioButton->isChecked()) {
        ps.filter = FilterSearch::RegisterAndRegular;
    } else if (ui->registerCheckBox->isChecked()) {
        ps.filter = FilterSearch::Register;
    } else if (ui->wordsOnlyRadioButton->isChecked()) {
        ps.filter = FilterSearch::WordsOnly;
    } else {
        ps.filter = FilterSearch::None;
    }

    switch (ui->startSearchComboBox->currentIndex()) {
    case 0:
        ps.period = PeriodSearch::WithTheEnd;
        break;
    case 1:
        ps.period = PeriodSearch::WithTheFirst;
        break;
    case 2:
        ps.period = PeriodSearch::AfterDate;
        break;
    case 3:
        ps.period = PeriodSearch::BeforeDate;
        break;
    default:
        ps.period = PeriodSearch::WithTheEnd;
        break;
    }

    ps.date = startDate;
    ps.isUpdate = isUpdate;
    isUpdate = false;

    return ps;
}

void SearchSettingsForm::updateStartDateLabel()
{
    ui->startDateLabel->setText(startDate.toString("dd.MM.yyyy"));
}

void SearchSettingsForm::onStartSearchSelected(const int index)
{
    isUpdate = true;
    if (index > 1) {
        ui->choiceDateButton->setEnabled(true);
        ui->startDateLabel->setEnabled(true);

        ui->choiceDateButton->setProperty("state", "green");
        ui->choiceDateButton->setStyleSheet(Style::getStylesheet(QStringLiteral(":/ui/chatForm/buttons.css")));

        ui->startDateLabel->setStyleSheet("QLabel{color: #000;}");

        if (startDate.isNull()) {
            startDate = QDate::currentDate();
            updateStartDateLabel();
        }

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
    isUpdate = true;
}

void SearchSettingsForm::onWordsOnlyClicked(const bool checked)
{
    isUpdate = true;
    if (checked) {
        ui->regularRadioButton->setChecked(false);
    }
}

void SearchSettingsForm::onRegularClicked(const bool checked)
{
    isUpdate = true;
    if (checked) {
        ui->wordsOnlyRadioButton->setChecked(false);
    }
}

void SearchSettingsForm::onChoiceDate()
{
    isUpdate = true;
    LoadHistoryDialog dlg;
    if (dlg.exec()) {
        startDate = dlg.getFromDate().date();
        updateStartDateLabel();
    }
}
