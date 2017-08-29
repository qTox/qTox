#include "aboutfriendform.h"
#include "ui_aboutfriendform.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>

AboutFriendForm::AboutFriendForm(const Friend* f, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AboutFriendForm)
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

    friendPk = f->getPublicKey();
    Settings& s = Settings::getInstance();
    QString dir = s.getAutoAcceptDir(friendPk);
    ui->autoacceptfile->setChecked(!dir.isEmpty());

    ui->autoacceptcall->setCurrentIndex(s.getAutoAcceptCall(friendPk));

    ui->selectSaveDir->setEnabled(ui->autoacceptfile->isChecked());
    ui->autogroupinvite->setChecked(s.getAutoGroupInvite(friendPk));

    if (ui->autoacceptfile->isChecked()) {
        ui->selectSaveDir->setText(s.getAutoAcceptDir(friendPk));
    }

    setWindowTitle(f->getDisplayedName());
    ui->userName->setText(f->getDisplayedName());
    ui->publicKey->setText(friendPk.toString());
    ui->publicKey->setCursorPosition(0); // scroll textline to left
    ui->note->setPlainText(Settings::getInstance().getContactNote(friendPk));

    QPixmap avatar = Nexus::getProfile()->loadAvatar(friendPk.toString());
    ui->statusMessage->setText(f->getStatusMessage());
    ui->avatar->setPixmap(avatar.isNull() ? QPixmap(":/img/contact_dark.svg") : avatar);
}

void AboutFriendForm::onAutoAcceptDirClicked()
{
    if (!ui->autoacceptfile->isChecked()) {
        ui->autoacceptfile->setChecked(false);
        Settings::getInstance().setAutoAcceptDir(friendPk, "");
        ui->selectSaveDir->setText(tr("Auto accept for this contact is disabled"));
    } else if (ui->autoacceptfile->isChecked()) {
        QString dir = QFileDialog::getExistingDirectory(
                    Q_NULLPTR, tr("Choose an auto accept directory", "popup title"), dir);

        if (dir.isEmpty()) {
            ui->autoacceptfile->setChecked(false);
            return; // user canellced
        }

        Settings::getInstance().setAutoAcceptDir(friendPk, dir);
        ui->selectSaveDir->setText(Settings::getInstance().getAutoAcceptDir(friendPk));
    }

    Settings::getInstance().saveGlobal();
    ui->selectSaveDir->setEnabled(ui->autoacceptfile->isChecked());
}

void AboutFriendForm::onAutoAcceptCallClicked()
{
    QFlag flag = QFlag(ui->autoacceptcall->currentIndex());
    Settings::getInstance().setAutoAcceptCall(friendPk, Settings::AutoAcceptCallFlags(flag));
    Settings::getInstance().savePersonal();
}

/**
 * @brief Sets the AutoGroupinvite status and saves the settings.
 */
void AboutFriendForm::onAutoGroupInvite()
{
    Settings::getInstance().setAutoGroupInvite(friendPk, ui->autogroupinvite->isChecked());
    Settings::getInstance().savePersonal();
}

void AboutFriendForm::onSelectDirClicked()
{
    QString dir = QFileDialog::getExistingDirectory(
                Q_NULLPTR, tr("Choose an auto accept directory", "popup title"), dir);

    ui->autoacceptfile->setChecked(true);
    Settings::getInstance().setAutoAcceptDir(friendPk, dir);
    Settings::getInstance().savePersonal();
}

/**
 * @brief Called when user clicks the bottom OK button, save all settings
 */
void AboutFriendForm::onAcceptedClicked()
{
    Settings::getInstance().setContactNote(friendPk, ui->note->toPlainText());
    Settings::getInstance().saveGlobal();
}

void AboutFriendForm::onRemoveHistoryClicked()
{
    History* history = Nexus::getProfile()->getHistory();
    if (history) {
        history->removeFriendHistory(friendPk.toString());
    }

    QMessageBox::information(this, tr("History removed"), tr("Chat history with %1 removed!")
                             .arg(ui->userName->text().toHtmlEscaped()), QMessageBox::Ok);
}

AboutFriendForm::~AboutFriendForm()
{
    delete ui;
}
