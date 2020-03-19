/*
    Copyright © 2019 by The qTox Project Contributors

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

#include "searchsettingsform.h"
#include "ui_searchsettingsform.h"
#include "src/persistence/settings.h"
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
    ui->choiceDateButton->setObjectName(QStringLiteral("choiceDateButton"));

    reloadTheme();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    connect(ui->startSearchComboBox, QOverload<int, const QString &>::of(&QComboBox::currentIndexChanged),
#else
    connect(ui->startSearchComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
#endif
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

    ps.time = startTime;
    ps.isUpdate = isUpdate;
    isUpdate = false;

    return ps;
}

void SearchSettingsForm::reloadTheme()
{
    ui->choiceDateButton->setStyleSheet(Style::getStylesheet(QStringLiteral("chatForm/buttons.css")));
    ui->startDateLabel->setStyleSheet(Style::getStylesheet(QStringLiteral("chatForm/labels.css")));
}

void SearchSettingsForm::updateStartDateLabel()
{
    ui->startDateLabel->setText(startTime.toString(Settings::getInstance().getDateFormat()));
}

void SearchSettingsForm::setUpdate(const bool isUpdate)
{
    this->isUpdate = isUpdate;
    emit updateSettings(isUpdate);
}

void SearchSettingsForm::onStartSearchSelected(const int index)
{
    if (index > 1) {
        ui->choiceDateButton->setEnabled(true);
        ui->startDateLabel->setEnabled(true);

        ui->choiceDateButton->setProperty("state", QStringLiteral("green"));
        ui->choiceDateButton->setStyleSheet(Style::getStylesheet(QStringLiteral("chatForm/buttons.css")));

        if (startTime.isNull()) {
            startTime = QDateTime::currentDateTime();
            updateStartDateLabel();
        }

    } else {
        ui->choiceDateButton->setEnabled(false);
        ui->startDateLabel->setEnabled(false);

        ui->choiceDateButton->setProperty("state", QString());
        ui->choiceDateButton->setStyleSheet(Style::getStylesheet(QStringLiteral("chatForm/buttons.css")));
    }

    setUpdate(true);
}

void SearchSettingsForm::onRegisterClicked(const bool checked)
{
    Q_UNUSED(checked)
    setUpdate(true);
}

void SearchSettingsForm::onWordsOnlyClicked(const bool checked)
{
    if (checked) {
        ui->regularRadioButton->setChecked(false);
    }

    setUpdate(true);
}

void SearchSettingsForm::onRegularClicked(const bool checked)
{
    if (checked) {
        ui->wordsOnlyRadioButton->setChecked(false);
    }

    setUpdate(true);
}

void SearchSettingsForm::onChoiceDate()
{
    LoadHistoryDialog dlg(LoadHistoryDialog::search);
    if (dlg.exec()) {
        startTime = dlg.getFromDate();
        updateStartDateLabel();
    }

    setUpdate(true);
}
