/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#include "privacyform.h"
#include "ui_privacysettings.h"

#include <QDebug>
#include <QFile>
#include <QMessageBox>

#include "src/core/core.h"
#include "src/nexus.h"
#include "src/persistence/history.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/widget/form/setpassworddialog.h"
#include "src/widget/form/settingswidget.h"
#include "src/widget/tool/recursivesignalblocker.h"
#include "src/widget/translator.h"
#include "src/widget/widget.h"

#include <chrono>
#include <random>

PrivacyForm::PrivacyForm(Core* core_, Settings& settings_, Style& style, Profile& profile_)
    : GenericForm(QPixmap(":/img/settings/privacy.png"), style)
    , bodyUI(new Ui::PrivacySettings)
    , core{core_}
    , settings{settings_}
    , profile{profile_}
{
    bodyUI->setupUi(this);

    // block all child signals during initialization
    const RecursiveSignalBlocker signalBlocker(this);

    eventsInit();
    Translator::registerHandler(std::bind(&PrivacyForm::retranslateUi, this), this);
}

PrivacyForm::~PrivacyForm()
{
    Translator::unregister(this);
    delete bodyUI;
}

void PrivacyForm::on_cbKeepHistory_stateChanged()
{
    settings.setEnableLogging(bodyUI->cbKeepHistory->isChecked());
    if (!bodyUI->cbKeepHistory->isChecked()) {
        emit clearAllReceipts();
        QMessageBox::StandardButton dialogDelHistory;
        dialogDelHistory =
            QMessageBox::question(nullptr, tr("Confirmation"),
                                  tr("Do you want to permanently delete all chat history?"),
                                  QMessageBox::Yes | QMessageBox::No);
        if (dialogDelHistory == QMessageBox::Yes) {
            profile.getHistory()->eraseHistory();
        }
    }
}

void PrivacyForm::on_cbTypingNotification_stateChanged()
{
    settings.setTypingNotification(bodyUI->cbTypingNotification->isChecked());
}

void PrivacyForm::on_nospamLineEdit_editingFinished()
{
    QString newNospam = bodyUI->nospamLineEdit->text();

    bool ok;
    uint32_t nospam = newNospam.toLongLong(&ok, 16);
    if (ok) {
        core->setNospam(nospam);
    }
}

void PrivacyForm::showEvent(QShowEvent* event)
{
    std::ignore = event;
    const Settings& s = settings;
    bodyUI->nospamLineEdit->setText(core->getSelfId().getNoSpamString());
    bodyUI->cbTypingNotification->setChecked(s.getTypingNotification());
    bodyUI->cbKeepHistory->setChecked(settings.getEnableLogging());
    bodyUI->blackListTextEdit->setText(s.getBlackList().join('\n'));
}

void PrivacyForm::on_randomNosapamButton_clicked()
{
    uint32_t newNospam{0};

    static std::mt19937 rng(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    newNospam = rng();

    core->setNospam(newNospam);
    bodyUI->nospamLineEdit->setText(core->getSelfId().getNoSpamString());
}

void PrivacyForm::on_nospamLineEdit_textChanged()
{
    QString str = bodyUI->nospamLineEdit->text();
    int curs = bodyUI->nospamLineEdit->cursorPosition();
    if (str.length() != 8) {
        str = QString("00000000").replace(0, str.length(), str);
        bodyUI->nospamLineEdit->setText(str);
        bodyUI->nospamLineEdit->setCursorPosition(curs);
    }
}

void PrivacyForm::on_blackListTextEdit_textChanged()
{
    const QStringList strlist = bodyUI->blackListTextEdit->toPlainText().split('\n');
    settings.setBlackList(strlist);
}

void PrivacyForm::retranslateUi()
{
    bodyUI->retranslateUi(this);
}
