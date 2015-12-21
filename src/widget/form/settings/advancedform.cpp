/*
    Copyright © 2014-2015 by The qTox Project

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

#include "ui_advancedsettings.h"

#include "advancedform.h"
#include "src/persistence/settings.h"
#include "src/persistence/db/plaindb.h"
#include "src/widget/translator.h"

AdvancedForm::AdvancedForm() :
    GenericForm(QPixmap(":/img/settings/general.png"))
{
    bodyUI = new Ui::AdvancedSettings;
    bodyUI->setupUi(this);

    bodyUI->cbMakeToxPortable->setChecked(Settings::getInstance().getMakeToxPortable());

    connect(bodyUI->cbMakeToxPortable, &QCheckBox::stateChanged, this, &AdvancedForm::onMakeToxPortableUpdated);
    connect(bodyUI->resetButton, SIGNAL(clicked()), this, SLOT(resetToDefault()));

    for (QCheckBox *cb : findChildren<QCheckBox*>()) // this one is to allow scrolling on checkboxes
    {
        cb->installEventFilter(this);
    }

    Translator::registerHandler(std::bind(&AdvancedForm::retranslateUi, this), this);
}

AdvancedForm::~AdvancedForm()
{
    Translator::unregister(this);
    delete bodyUI;
}

void AdvancedForm::onMakeToxPortableUpdated()
{
    Settings::getInstance().setMakeToxPortable(bodyUI->cbMakeToxPortable->isChecked());
}

void AdvancedForm::resetToDefault()
{
}

bool AdvancedForm::eventFilter(QObject *o, QEvent *e)
{
    if ((e->type() == QEvent::Wheel) &&
         (qobject_cast<QAbstractSpinBox*>(o) || qobject_cast<QCheckBox*>(o)))
    {
        e->ignore();
        return true;
    }
    return QWidget::eventFilter(o, e);
}

void AdvancedForm::retranslateUi()
{
    bodyUI->retranslateUi(this);
}
