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
#include "src/core/core.h"
#include "src/widget/gui.h"

AdvancedForm::AdvancedForm() :
    GenericForm(QPixmap(":/img/settings/general.png"))
{
    bodyUI = new Ui::AdvancedSettings;
    bodyUI->setupUi(this);

    bodyUI->cbMakeToxPortable->setChecked(Settings::getInstance().getMakeToxPortable());

    connect(bodyUI->cbMakeToxPortable, &QCheckBox::stateChanged, this, &AdvancedForm::onMakeToxPortableUpdated);
    connect(bodyUI->addDevBtn, &QPushButton::clicked, this, &AdvancedForm::onAddDeviceClicked);
    connect(bodyUI->delDevBtn, &QPushButton::clicked, this, &AdvancedForm::onRemoveDeviceClicked);

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

void AdvancedForm::showEvent(QShowEvent *event)
{
    fillDeviceList();
}

void AdvancedForm::retranslateUi()
{
    bodyUI->retranslateUi(this);
}

void AdvancedForm::fillDeviceList()
{
    QVector<ToxDevice> devices = Core::getInstance()->getDeviceList();

    bodyUI->devsTable->clearContents();
    bodyUI->devsTable->setRowCount(0);

    for (auto&& dev : devices)
    {
        int newRow = bodyUI->devsTable->rowCount();
        bodyUI->devsTable->insertRow(newRow);
        bodyUI->devsTable->setItem(newRow, 0, new QTableWidgetItem(dev.name));
        if (dev.status == DeviceStatus::Pending)
            bodyUI->devsTable->setItem(newRow, 1, new QTableWidgetItem("Pending"));
        else if (dev.status == DeviceStatus::Confirmed)
            bodyUI->devsTable->setItem(newRow, 1, new QTableWidgetItem("Offline"));
        else if (dev.status == DeviceStatus::Online)
            bodyUI->devsTable->setItem(newRow, 1, new QTableWidgetItem("Online"));
        bodyUI->devsTable->setItem(newRow, 2, new QTableWidgetItem(QString(dev.pk.toHex()).toUpper()));
    }
}

void AdvancedForm::onAddDeviceClicked()
{
    QString toxId = GUI::textDialog("Add Device", "Tox ID");
    if (!ToxId::isToxId(toxId))
        GUI::showError("Error", "Invalid Tox ID");
    QByteArray pk = QByteArray::fromHex(ToxId(toxId).publicKey.toUtf8());

    if (!Core::getInstance()->addDevice("Unnamed", pk))
        GUI::showError("Error", "Failed to add the device");

    fillDeviceList();
}

void AdvancedForm::onRemoveDeviceClicked()
{
    QList<QTableWidgetSelectionRange> ranges = bodyUI->devsTable->selectedRanges();
    if (ranges.isEmpty())
        return;
    QByteArray pk = QByteArray::fromHex(bodyUI->devsTable->item(ranges[0].topRow(), 2)->text().toUtf8());

    if (!GUI::askQuestion("Are you sure?", "The device will be blacklisted permanently.\nContinue?"))
        return;

    if (!Core::getInstance()->removeDevice(pk))
    {
        GUI::showError("Error", "Failed to remove the device");
        return;
    }

    fillDeviceList();
}
