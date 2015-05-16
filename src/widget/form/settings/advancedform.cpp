/*
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

#include "ui_advancedsettings.h"

#include "advancedform.h"
#include "src/historykeeper.h"
#include "src/misc/settings.h"
#include "src/misc/db/plaindb.h"

AdvancedForm::AdvancedForm(SettingsWidget *parent) :
    GenericForm(tr("Advanced"), QPixmap(":/img/settings/general.png"))
{
    bodyUI = new Ui::AdvancedSettings;
    bodyUI->setupUi(this);

    bodyUI->dbLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    bodyUI->dbLabel->setOpenExternalLinks(true);

    bodyUI->cbMakeToxPortable->setChecked(Settings::getInstance().getMakeToxPortable());
    bodyUI->syncTypeComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    bodyUI->syncTypeComboBox->addItems({tr("FULL - very safe, slowest (recommended)"),
                                        tr("NORMAL - almost as safe as FULL, about 20% faster than FULL"),
                                        tr("OFF - disables all safety, when something goes wrong your history may be lost, fastest (not recommended)")
                                       });
    int index = 2 - static_cast<int>(Settings::getInstance().getDbSyncType());
    bodyUI->syncTypeComboBox->setCurrentIndex(index);

    connect(bodyUI->cbMakeToxPortable, &QCheckBox::stateChanged, this, &AdvancedForm::onMakeToxPortableUpdated);
    connect(bodyUI->syncTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onDbSyncTypeUpdated()));
    connect(bodyUI->resetButton, SIGNAL(clicked()), this, SLOT(resetToDefault()));

    for (QComboBox* cb : findChildren<QComboBox*>())
    {
            cb->installEventFilter(this);
            cb->setFocusPolicy(Qt::StrongFocus);
    }
}

AdvancedForm::~AdvancedForm()
{
    delete bodyUI;
}

void AdvancedForm::onMakeToxPortableUpdated()
{
    Settings::getInstance().setMakeToxPortable(bodyUI->cbMakeToxPortable->isChecked());
}

void AdvancedForm::onDbSyncTypeUpdated()
{
    int index = 2 - bodyUI->syncTypeComboBox->currentIndex();
    Settings::getInstance().setDbSyncType(index);
    HistoryKeeper::getInstance()->setSyncType(Settings::getInstance().getDbSyncType());
}

void AdvancedForm::resetToDefault()
{
    int index = 2 - static_cast<int>(Db::syncType::stFull);
    bodyUI->syncTypeComboBox->setCurrentIndex(index);
    onDbSyncTypeUpdated();
}

bool AdvancedForm::eventFilter(QObject *o, QEvent *e)
{
    if ((e->type() == QEvent::Wheel) &&
         (qobject_cast<QComboBox*>(o) || qobject_cast<QAbstractSpinBox*>(o) ))
    {
        e->ignore();
        return true;
    }
    return QWidget::eventFilter(o, e);
}
