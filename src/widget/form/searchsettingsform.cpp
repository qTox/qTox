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

SearchSettingsForm::SearchSettingsForm(Settings& settings_, Style& style_, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SearchSettingsForm)
    , settings{settings_}
    , style{style_}
{
    ui->setupUi(this);

    ui->choiceDateButton->setEnabled(false);
    ui->startDateLabel->setEnabled(false);

    ui->choiceDateButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->choiceDateButton->setObjectName(QStringLiteral("choiceDateButton"));

    reloadTheme();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    connect(ui->startSearchComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
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

    ps.date = startDate;
    ps.isUpdate = isUpdate;
    isUpdate = false;

    return ps;
}

void SearchSettingsForm::reloadTheme()
{
    ui->choiceDateButton->setStyleSheet(style.getStylesheet(QStringLiteral("chatForm/buttons.css"), settings));
    ui->startDateLabel->setStyleSheet(style.getStylesheet(QStringLiteral("chatForm/labels.css"), settings));
}

void SearchSettingsForm::updateStartDateLabel()
{
    ui->startDateLabel->setText(startDate.toString(settings.getDateFormat()));
}

void SearchSettingsForm::setUpdate(const bool isUpdate_)
{
    isUpdate = isUpdate_;
    emit updateSettings(isUpdate_);
}

void SearchSettingsForm::onStartSearchSelected(const int index)
{
    if (index > 1) {
        ui->choiceDateButton->setEnabled(true);
        ui->startDateLabel->setEnabled(true);

        ui->choiceDateButton->setProperty("state", QStringLiteral("green"));
        ui->choiceDateButton->setStyleSheet(style.getStylesheet(QStringLiteral("chatForm/buttons.css"), settings));

        if (startDate.isNull()) {
            startDate = QDate::currentDate();
            updateStartDateLabel();
        }

    } else {
        ui->choiceDateButton->setEnabled(false);
        ui->startDateLabel->setEnabled(false);

        ui->choiceDateButton->setProperty("state", QString());
        ui->choiceDateButton->setStyleSheet(style.getStylesheet(QStringLiteral("chatForm/buttons.css"), settings));
    }

    setUpdate(true);
}

void SearchSettingsForm::onRegisterClicked(const bool checked)
{
    std::ignore = checked;
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
    LoadHistoryDialog dlg;
    dlg.setTitle(tr("Select Date Dialog"));
    dlg.setInfoLabel(tr("Select a date"));
    if (dlg.exec()) {
        startDate = dlg.getFromDate().date();
        updateStartDateLabel();
    }

    setUpdate(true);
}
