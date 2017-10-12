#include "aboutfriendform.h"
#include "ui_aboutfriendform.h"

#include <QFileDialog>
#include <QMessageBox>

AboutFriendForm::AboutFriendForm(QPointer<IAboutFriend> about, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AboutFriendForm)
    , about{about}
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

    const QString dir = about->getAutoAcceptDir();
    ui->autoacceptfile->setChecked(!dir.isEmpty());

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
    ui->publicKey->setText(about->getPublicKey());
    ui->publicKey->setCursorPosition(0); // scroll textline to left
    ui->note->setPlainText(about->getNote());
    ui->statusMessage->setText(about->getStatusMessage());
    ui->avatar->setPixmap(about->getAvatar());
}

void AboutFriendForm::onAutoAcceptDirClicked()
{
    if (!ui->autoacceptfile->isChecked()) {
        ui->autoacceptfile->setChecked(false);
        about->setAutoAcceptDir("");
        ui->selectSaveDir->setText(tr("Auto accept for this contact is disabled"));
    } else if (ui->autoacceptfile->isChecked()) {
        QString dir = about->getAutoAcceptDir();
        dir = QFileDialog::getExistingDirectory(
                    Q_NULLPTR, tr("Choose an auto accept directory", "popup title"), dir);

        if (dir.isEmpty()) {
            ui->autoacceptfile->setChecked(false);
            return; // user canellced
        }

        about->setAutoAcceptDir(dir);
        ui->selectSaveDir->setText(about->getAutoAcceptDir());
    }

    ui->selectSaveDir->setEnabled(ui->autoacceptfile->isChecked());
}

void AboutFriendForm::onAutoAcceptCallClicked()
{
    const int index = ui->autoacceptcall->currentIndex();
    const IAboutFriend::AutoAcceptCall flag = static_cast<IAboutFriend::AutoAcceptCall>(index);
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
    QString dir = about->getAutoAcceptDir();
    dir = QFileDialog::getExistingDirectory(
                Q_NULLPTR, tr("Choose an auto accept directory", "popup title"), dir);

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
    about->clearHistory();

    QMessageBox::information(this, tr("History removed"), tr("Chat history with %1 removed!")
                             .arg(about->getName().toHtmlEscaped()), QMessageBox::Ok);
}

AboutFriendForm::~AboutFriendForm()
{
    delete ui;
}
