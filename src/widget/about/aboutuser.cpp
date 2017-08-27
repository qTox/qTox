#include "aboutuser.h"
#include "ui_aboutuser.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

AboutUser::AboutUser(const Friend* f, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AboutUser)
{
    ui->setupUi(this);
    ui->label_4->hide();
    ui->aliases->hide();

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &AboutUser::onAcceptedClicked);
    connect(ui->autoacceptfile, &QCheckBox::clicked, this, &AboutUser::onAutoAcceptDirClicked);
    connect(ui->autoacceptcall, SIGNAL(activated(int)), this, SLOT(onAutoAcceptCallClicked(void)));
    connect(ui->autogroupinvite, &QCheckBox::clicked, this, &AboutUser::onAutoGroupInvite);
    connect(ui->selectSaveDir, &QPushButton::clicked, this, &AboutUser::onSelectDirClicked);
    connect(ui->removeHistory, &QPushButton::clicked, this, &AboutUser::onRemoveHistoryClicked);

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

/**
 * @brief Show window for select auto accept directory
 * @param friendPk
 * @return
 */
static bool selectAutoAcceptDirectory(const ToxPk& friendPk, QPushButton* selectSaveDir)
{
    QString dir = Settings::getInstance().getAutoAcceptDir(friendPk);
    dir = QFileDialog::getExistingDirectory(
                Q_NULLPTR, AboutUser::tr("Choose an auto accept directory", "popup title"), dir);

    if (dir.isEmpty()) {
        return false; // user canellced
    }

    Settings::getInstance().setAutoAcceptDir(friendPk, dir);
    selectSaveDir->setText(dir);
    return true;
}

void AboutUser::onAutoAcceptDirClicked()
{
    bool autoAccept = ui->autoacceptfile->isChecked();
    if (autoAccept) {
        if (!selectAutoAcceptDirectory(friendPk, ui->selectSaveDir)) {
            ui->autoacceptfile->setChecked(false);
        }
    } else {
        Settings::getInstance().setAutoAcceptDir(friendPk, "");
        ui->selectSaveDir->setText(tr("Auto accept for this contact is disabled"));
    }

    Settings::getInstance().savePersonal();
    ui->selectSaveDir->setEnabled(autoAccept);
}

void AboutUser::onAutoAcceptCallClicked()
{
    QFlag flag = QFlag(ui->autoacceptcall->currentIndex());
    Settings::getInstance().setAutoAcceptCall(friendPk, Settings::AutoAcceptCallFlags(flag));
    Settings::getInstance().savePersonal();
}

/**
 * @brief Sets the AutoGroupinvite status and saves the settings.
 */
void AboutUser::onAutoGroupInvite()
{
    Settings::getInstance().setAutoGroupInvite(friendPk, ui->autogroupinvite->isChecked());
    Settings::getInstance().savePersonal();
}

void AboutUser::onSelectDirClicked()
{
    selectAutoAcceptDirectory(friendPk, ui->selectSaveDir);

    Settings::getInstance().savePersonal();
}

/**
 * @brief Called when user clicks the bottom OK button, save all settings
 */
void AboutUser::onAcceptedClicked()
{
    Settings::getInstance().setContactNote(friendPk, ui->note->toPlainText());
    Settings::getInstance().saveGlobal();
}

void AboutUser::onRemoveHistoryClicked()
{
    History* history = Nexus::getProfile()->getHistory();
    if (history) {
        history->removeFriendHistory(friendPk.toString());
    }

    QMessageBox::information(this, tr("History removed"), tr("Chat history with %1 removed!")
                             .arg(ui->userName->text().toHtmlEscaped()), QMessageBox::Ok);
}

AboutUser::~AboutUser()
{
    delete ui;
}
