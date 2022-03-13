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

#include "aboutfriendform.h"
#include "src/widget/tool/imessageboxmanager.h"
#include "ui_aboutfriendform.h"
#include "src/core/core.h"
#include "src/widget/style.h"

#include <QFileDialog>
#include <QMessageBox>

namespace {
QString getAutoAcceptDir(const QString& dir)
{
    //: popup title
    const QString title = AboutFriendForm::tr("Choose an auto-accept directory");
    return QFileDialog::getExistingDirectory(Q_NULLPTR, title, dir);
}

} // namespace

AboutFriendForm::AboutFriendForm(std::unique_ptr<IAboutFriend> about_,
    Settings& settings_, Style& style_, IMessageBoxManager& messageBoxManager_, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AboutFriendForm)
    , about{std::move(about_)}
    , settings{settings_}
    , style{style_}
    , messageBoxManager{messageBoxManager_}
{
    ui->setupUi(this);
    ui->label_4->hide();
    ui->aliases->hide();

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &AboutFriendForm::onAcceptedClicked);
    connect(ui->autoacceptfile, &QCheckBox::clicked, this, &AboutFriendForm::onAutoAcceptDirClicked);
    connect(ui->autoacceptcall, SIGNAL(activated(int)), this, SLOT(onAutoAcceptCallClicked(void)));
    connect(ui->autogroupinvite, &QCheckBox::clicked, this, &AboutFriendForm::onAutoGroupInvite);
    connect(ui->selectSaveDir, &QPushButton::clicked, this, &AboutFriendForm::onSelectDirClicked);
    connect(ui->removeHistory, &QPushButton::clicked, this, &AboutFriendForm::onRemoveHistoryClicked);
    about->connectTo_autoAcceptDirChanged(this, [=](const QString& dir){ onAutoAcceptDirChanged(dir); });

    const QString dir = about->getAutoAcceptDir();
    ui->autoacceptfile->setChecked(!dir.isEmpty());

    ui->removeHistory->setEnabled(about->isHistoryExistence());

    const int index = static_cast<int>(about->getAutoAcceptCall());
    ui->autoacceptcall->setCurrentIndex(index);

    ui->selectSaveDir->setEnabled(ui->autoacceptfile->isChecked());
    ui->autogroupinvite->setChecked(about->getAutoGroupInvite());

    if (ui->autoacceptfile->isChecked()) {
        ui->selectSaveDir->setText(about->getAutoAcceptDir());
    }

    const QString name = about->getName();
    setWindowTitle(name);
    ui->userName->setText(name);
    ui->publicKey->setText(about->getPublicKey().toString());
    ui->publicKey->setCursorPosition(0); // scroll textline to left
    ui->note->setPlainText(about->getNote());
    ui->statusMessage->setText(about->getStatusMessage());
    ui->avatar->setPixmap(about->getAvatar());

    connect(&style, &Style::themeReload, this, &AboutFriendForm::reloadTheme);

    reloadTheme();
}

void AboutFriendForm::onAutoAcceptDirClicked()
{
    const QString dir = [&]{
        if (!ui->autoacceptfile->isChecked()) {
            return QString{};
        }

        return getAutoAcceptDir(about->getAutoAcceptDir());
    }();

    about->setAutoAcceptDir(dir);
}

void AboutFriendForm::reloadTheme()
{
    setStyleSheet(style.getStylesheet("window/general.css", settings));
}

void AboutFriendForm::onAutoAcceptDirChanged(const QString& path)
{
    const bool enabled = !path.isNull();
    ui->autoacceptfile->setChecked(enabled);
    ui->selectSaveDir->setEnabled(enabled);
    ui->selectSaveDir->setText(enabled ? path : tr("Auto-accept for this contact is disabled"));
}


void AboutFriendForm::onAutoAcceptCallClicked()
{
    const int index = ui->autoacceptcall->currentIndex();
    const IFriendSettings::AutoAcceptCallFlags flag{index};
    about->setAutoAcceptCall(flag);
}

/**
 * @brief Sets the AutoGroupinvite status and saves the settings.
 */
void AboutFriendForm::onAutoGroupInvite()
{
    about->setAutoGroupInvite(ui->autogroupinvite->isChecked());
}

void AboutFriendForm::onSelectDirClicked()
{
    const QString dir = getAutoAcceptDir(about->getAutoAcceptDir());
    about->setAutoAcceptDir(dir);
}

/**
 * @brief Called when user clicks the bottom OK button, save all settings
 */
void AboutFriendForm::onAcceptedClicked()
{
    about->setNote(ui->note->toPlainText());
}

void AboutFriendForm::onRemoveHistoryClicked()
{
   const bool retYes = messageBoxManager.askQuestion(tr("Confirmation"),
                                   tr("Are you sure to remove %1 chat history?").arg(about->getName()),
                                   /* defaultAns = */ false, /* warning = */ true, /* yesno = */ true);
    if (!retYes) {
        return;
    }

   const bool result = about->clearHistory();

    if (!result) {
        messageBoxManager.showWarning(tr("History removed"),
                         tr("Failed to remove chat history with %1!").arg(about->getName()).toHtmlEscaped());
        return;
    }

    emit histroyRemoved();

    ui->removeHistory->setEnabled(false); // For know clearly to has removed the history
}

AboutFriendForm::~AboutFriendForm()
{
    delete ui;
}
